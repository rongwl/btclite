#include "msg_process.h"

#include <event2/buffer.h>

#include "protocol/ping.h"
#include "protocol/version.h"


using namespace btclite::network::protocol;

namespace btclite {
namespace network {
namespace msgprocess{

MessageData *MsgDataFactory(const MessageHeader& header, const uint8_t *raw)
{
    if (header.command() == kMsgVersion) {
        return new Version(raw, header.payload_length());
    }
    else if (header.command() == kMsgPing) {
        return new Ping(raw);
    }
    
    return nullptr;
}

bool ParseMsg(std::shared_ptr<Node> src_node)
{
    struct evbuffer *buf;
    uint8_t *raw = nullptr;
    
    if (!src_node->bev())
        return false;
    
    if (nullptr == (buf = bufferevent_get_input(src_node->mutable_bev())))
        return false;

    if (src_node->disconnected())
        return false;
    
    if (nullptr == (raw = evbuffer_pullup(buf, MessageHeader::kSize)))
        return false;
    
    while (raw) {
        MessageHeader header(raw);
        if (!header.IsValid()) {
            BTCLOG(LOG_LEVEL_ERROR) << "Received invalid message header from peer " << src_node->id();
            return false;
        }
        evbuffer_drain(buf, MessageHeader::kSize);
        
        raw = evbuffer_pullup(buf, header.payload_length());
        MessageData *message = MsgDataFactory(header, raw);
        if (!message) {
            BTCLOG(LOG_LEVEL_ERROR) << "Prasing message data from peer " << src_node->id() << " failed.";
            return false;
        }
        if (!message->IsValid()) {
            BTCLOG(LOG_LEVEL_ERROR) << "Received invalid message data from peer " << src_node->id();
            return false;
        }
        evbuffer_drain(buf, header.payload_length());
        
        if (!message->RecvHandler(src_node)) {
            delete message;
            return false;
        }
        delete message;
        
        raw = evbuffer_pullup(buf, MessageHeader::kSize);
    }

    return true;
}

bool SendVerMsg(std::shared_ptr<Node> dst_node)
{
    ServiceFlags services = dst_node->services();
    uint32_t start_height = dst_node->start_height();
    btclite::network::NetAddr addr_recv(dst_node->addr());
    btclite::network::NetAddr addr_from;
    
    addr_from.mutable_proto_addr()->set_services(services);
    Version ver_msg(kProtocolVersion, services, btclite::utility::util_time::GetTimeSeconds(),
                    std::move(addr_recv), std::move(addr_from), dst_node->local_host_nonce(),
                    BTCLITE_USER_AGENT, start_height, true);

    if (!SendMsg(ver_msg, dst_node))
        return false;

    BTCLOG(LOG_LEVEL_INFO) << "Send version message: version " << kProtocolVersion 
                           << ", start_height=" << start_height << ", addr_recv=" << addr_recv.ToString() 
                           << ", peer=" << dst_node->id();
    
    return true;
}

} // namespace msgprocess
} // namespace network
} // namespace btclite
