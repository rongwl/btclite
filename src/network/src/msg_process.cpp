#include "msg_process.h"

#include <event2/buffer.h>

#include "chain_state.h"
#include "protocol/address.h"
#include "protocol/getaddr.h"
#include "protocol/inventory.h"
#include "protocol/ping.h"
#include "protocol/pong.h"
#include "protocol/reject.h"
#include "protocol/send_headers.h"
#include "protocol/send_compact.h"
#include "protocol/verack.h"
#include "protocol/version.h"


namespace btclite {
namespace network {

using namespace protocol;

bool ParseMsgData(const uint8_t *raw, std::shared_ptr<Node> src_node, 
                  const MessageHeader& header, const Params& params)
{
    std::vector<uint8_t> vec;
    util::ByteSource<std::vector<uint8_t> > byte_source(vec);
    
    if (!header.IsValid()) {
        BTCLOG(LOG_LEVEL_ERROR) << "Received invalid " << header.command()
                                << " message header";
        return false;    
    }
    
    vec.reserve(header.payload_length());
    vec.assign(raw, raw + header.payload_length());
    
    if (header.command() == msg_command::kMsgVersion) {
        Version version;
        version.Deserialize(byte_source);
        return HandleMsgData(src_node, header, version, 
                             params.msg_magic(), params.advertise_local_addr());
    }
    else if (header.command() == msg_command::kMsgVerack) {
        Verack verack;
        verack.Deserialize(byte_source);
        return HandleMsgData(src_node, header, verack,
                             params.msg_magic(), params.advertise_local_addr());
    }
    else if (header.command() == msg_command::kMsgAddr) {
        Addr addr;
        addr.Deserialize(byte_source);
        return HandleMsgData(src_node, header, addr);
    }
    else if (header.command() == msg_command::kMsgInv) {
        Inv inv;
        inv.Deserialize(byte_source);
        return HandleMsgData(src_node, header, inv);
    }
    else if (header.command() == msg_command::kMsgGetAddr) {
        GetAddr getaddr;
        getaddr.Deserialize(byte_source);
        return HandleMsgData(src_node, header, getaddr);
    }
    else if (header.command() == msg_command::kMsgPing) {
        Ping ping(0, src_node->protocol().version());
        ping.Deserialize(byte_source);
        return HandleMsgData(src_node, header, ping, params.msg_magic());
    }
    else if (header.command() == msg_command::kMsgPong) {
        Pong pong;
        pong.Deserialize(byte_source);
        return HandleMsgData(src_node, header, pong);
    }
    else if (header.command() == msg_command::kMsgReject) {
        Reject reject;
        reject.Deserialize(byte_source);
        return HandleMsgData(src_node, header, reject);
    }
    else if (header.command() == msg_command::kMsgSendHeaders) {
        SendHeaders send_headers;
        send_headers.Deserialize(byte_source);
        return HandleMsgData(src_node, header, send_headers);
    }
    else if (header.command() == msg_command::kMsgSendCmpct) {
        SendCmpct send_compact;
        send_compact.Deserialize(byte_source);
        return HandleMsgData(src_node, header, send_compact);
    }
    else {
        BTCLOG(LOG_LEVEL_WARNING) << "Rececived unknown message: "
                                  << header.command();
        return false;
    }
    
    return true;
}

bool ParseMsg(std::shared_ptr<Node> src_node, const Params& params)
{
    struct evbuffer *buf;
    uint8_t *raw = nullptr;
    bool ret = true;
    
    if (!src_node->connection().bev())
        return false;
    
    if (nullptr == (buf = bufferevent_get_input(
                              src_node->mutable_connection()->mutable_bev())))
        return false;

    if (src_node->connection().connection_state() == NodeConnection::kDisconnected)
        return false;
    
    if (nullptr == (raw = evbuffer_pullup(buf, MessageHeader::kSize)))
        return false;
    
    while (raw) {
        // construct msg header from raw
        MessageHeader header(raw);
        evbuffer_drain(buf, MessageHeader::kSize);
        if (header.payload_length() > kMaxMessageSize) {
            BTCLOG(LOG_LEVEL_ERROR) << "Oversized message from peer " << src_node->id()
                                    << ", disconnecting";
            src_node->mutable_connection()->set_connection_state(NodeConnection::kDisconnected);
            return false;
        }
        if (header.magic() != params.msg_magic()) {
            BTCLOG(LOG_LEVEL_ERROR) << "Invalid message magic " << header.magic()
                                    << " from peer " << src_node->id() << ", disconnecting";
            src_node->mutable_connection()->set_connection_state(NodeConnection::kDisconnected);
            return false;
        }
        
        if (header.payload_length() > 0) {
            if (nullptr == (raw = evbuffer_pullup(buf, header.payload_length())))
                return false;
        }
        
        // construct msg data from raw
        ret &= ParseMsgData(raw, src_node, header, params);
        evbuffer_drain(buf, header.payload_length());     
      
        raw = evbuffer_pullup(buf, MessageHeader::kSize);
    }

    return ret;
}

bool SendVersion(std::shared_ptr<Node> dst_node, uint32_t magic)
{
    ServiceFlags services = dst_node->protocol().services();
    uint32_t start_height = chain::SingletonChainState::GetInstance().active_chain().Height();
    NetAddr addr_recv(dst_node->connection().addr());
    NetAddr addr_from;
    
    addr_from.set_services(services);
    Version ver_msg(kProtocolVersion, services,
                    util::GetTimeSeconds(),
                    addr_recv, addr_from,
                    dst_node->local_host_nonce(), 
                    std::move(protocol::FormatUserAgent()),
                    start_height, true);

    if (!SendMsg(ver_msg, magic, dst_node)) {
        return false;
    }
    
    BTCLOG(LOG_LEVEL_INFO) << "Send version message: version " << kProtocolVersion 
                           << ", start_height=" << start_height 
                           << ", addr_recv=" << addr_recv.ToString() 
                           << ", peer=" << dst_node->id();
    
    return true;
}

bool SendAddr(std::shared_ptr<Node> dst_node)
{
    return true;
}


} // namespace network
} // namespace btclite
