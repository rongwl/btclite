#include "msg_process.h"

#include <event2/buffer.h>

#include "chain_state.h"
#include "protocol/addr.h"
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

using namespace std::placeholders;
using namespace protocol;

bool ParseMsgData(const uint8_t *raw, std::shared_ptr<Node> src_node, 
                  const MessageHeader& header, const Params& params,
                  const LocalService& local_service, Peers *ppeers)
{
    std::vector<uint8_t> vec;
    util::ByteSource<std::vector<uint8_t> > byte_source(vec);
    
    if (!header.IsValid()) {
        return false;    
    }
    
    vec.reserve(header.payload_length());
    vec.assign(raw, raw + header.payload_length());
    
    if (header.command() == msg_command::kMsgVersion) {
        Version version;
        version.Deserialize(byte_source);
        auto recv_handler = std::bind(&Version::RecvHandler, &version,
                                      _1, params.msg_magic(), 
                                      params.advertise_local_addr(),
                                      std::ref(local_service), ppeers);
        return HandleMsgData(src_node, header, version, recv_handler);
    }
    else if (header.command() == msg_command::kMsgVerack) {
        Verack verack;
        verack.Deserialize(byte_source);
        auto recv_handler = std::bind(&Verack::RecvHandler, &verack,
                                      _1, params.msg_magic(),
                                      params.advertise_local_addr(),
                                      std::ref(local_service));
        return HandleMsgData(src_node, header, verack, recv_handler);
    }
    else if (header.command() == msg_command::kMsgAddr) {
        Addr addr;
        addr.Deserialize(byte_source);
        return HandleMsgData(src_node, header, addr,
                             std::bind(&Addr::RecvHandler, &addr, _1));
    }
    else if (header.command() == msg_command::kMsgInv) {
        Inv inv;
        inv.Deserialize(byte_source);
        return HandleMsgData(src_node, header, inv,
                             std::bind(&Inv::RecvHandler, &inv, _1));
    }
    else if (header.command() == msg_command::kMsgGetAddr) {
        GetAddr getaddr;
        getaddr.Deserialize(byte_source);
        return HandleMsgData(src_node, header, getaddr,
                             std::bind(&GetAddr::RecvHandler, &getaddr, _1));
    }
    else if (header.command() == msg_command::kMsgPing) {
        Ping ping(0, src_node->protocol().version);
        ping.Deserialize(byte_source);
        auto recv_handler = std::bind(&Ping::RecvHandler, &ping, _1,
                                      params.msg_magic());
        return HandleMsgData(src_node, header, ping, recv_handler);
    }
    else if (header.command() == msg_command::kMsgPong) {
        Pong pong;
        pong.Deserialize(byte_source);
        return HandleMsgData(src_node, header, pong,
                             std::bind(&Pong::RecvHandler, &pong, _1));
    }
    else if (header.command() == msg_command::kMsgReject) {
        Reject reject;
        reject.Deserialize(byte_source);
        return HandleMsgData(src_node, header, reject,
                             std::bind(&Reject::RecvHandler, &reject, _1));
    }
    else if (header.command() == msg_command::kMsgSendHeaders) {
        SendHeaders send_headers;
        send_headers.Deserialize(byte_source);
        return HandleMsgData(src_node, header, send_headers,
                             std::bind(&SendHeaders::RecvHandler, &send_headers, _1));
    }
    else if (header.command() == msg_command::kMsgSendCmpct) {
        SendCmpct send_compact;
        send_compact.Deserialize(byte_source);
        return HandleMsgData(src_node, header, send_compact,
                             std::bind(&SendCmpct::RecvHandler, &send_compact, _1));
    }
    else {
        BTCLOG(LOG_LEVEL_WARNING) << "Rececived unknown message: "
                                  << header.command();
        return false;
    }
    
    return true;
}

bool ParseMsg(std::shared_ptr<Node> src_node, const Params& params, 
              const LocalService& local_service,Peers *ppeers)
{
    struct evbuffer *buf;
    uint8_t *raw = nullptr;
    bool ret = true;
    
    if (!src_node->connection().bev())
        return false;
    
    if (nullptr == (buf = bufferevent_get_input(
                              src_node->mutable_connection()->mutable_bev())))
        return false;

    if (src_node->connection().IsDisconnected())
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
            src_node->mutable_connection()->Disconnect();
            return false;
        }
        if (header.magic() != params.msg_magic()) {
            BTCLOG(LOG_LEVEL_ERROR) << "Invalid message magic " << header.magic()
                                    << " from peer " << src_node->id() << ", disconnecting";
            src_node->mutable_connection()->Disconnect();
            return false;
        }
        
        if (header.payload_length() > 0) {
            if (nullptr == (raw = evbuffer_pullup(buf, header.payload_length())))
                return false;
        }
        
        // construct msg data from raw
        ret &= ParseMsgData(raw, src_node, header, params, local_service, ppeers);
        evbuffer_drain(buf, header.payload_length()); 
      
        raw = evbuffer_pullup(buf, MessageHeader::kSize);
    }

    return ret;
}

bool SendVersion(std::shared_ptr<Node> dst_node, uint32_t magic)
{
    ServiceFlags services = dst_node->services();
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
