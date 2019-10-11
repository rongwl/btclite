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
    uint8_t *raw;
    
    if (!src_node->bev())
        return false;
    
    if (nullptr == (buf = bufferevent_get_input(src_node->mutable_bev())))
        return false;
    
    if (src_node->disconnected())
        return false;
    
    raw = evbuffer_pullup(buf, MessageHeader::kSize);
    while (raw) {
        MessageHeader header(raw);
        
        if (!header.IsValid()) {
            BTCLOG(LOG_LEVEL_ERROR) << "Received invalid message header from peer " << src_node->id();
            return false;
        }
        
        raw = evbuffer_pullup(buf, header.payload_length());
        MessageData *message = MsgDataFactory(header, raw);
        if (!message) {
            BTCLOG(LOG_LEVEL_ERROR) << "Prasing message data from peer " << src_node->id() << " failed.";
            return false;
        }
        
        message->RecvHandler(src_node);
        delete message;
        
        raw = evbuffer_pullup(buf, MessageHeader::kSize);
    }
    
    return true;
}

bool SendVerMsg(std::shared_ptr<Node> dst_node)
{
    uint64_t nonce = dst_node->local_host_nonce();
    int nNodeStartingHeight = dst_node->start_height();
    btclite::network::NetAddr addr_recv(dst_node->addr()), addr_from();
    
    addr_from.mutable_proto_addr()->set_services(dst_node->services());
    
    return true;
}

} // namespace msgprocess
} // namespace network
} // namespace btclite
