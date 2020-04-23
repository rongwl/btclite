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

MessageData *MsgDataFactory(const uint8_t *raw, const MessageHeader& header, 
                            uint32_t protocol_version)
{    
    std::vector<uint8_t> vec;
    util::ByteSource<std::vector<uint8_t> > byte_source(vec);
    MessageData *msg = nullptr;
    
    if (!header.IsValid()) {
        BTCLOG(LOG_LEVEL_ERROR) << "Received invalid " << header.command()
                                << " message header";
        return nullptr;    
    }
    
    vec.reserve(header.payload_length());
    vec.assign(raw, raw + header.payload_length());
    
    if (header.command() == msg_command::kMsgVersion) {        
        Version *version = new Version();
        version->Deserialize(byte_source);
        if (header.checksum() != version->GetHash().GetLow32()) {
            BTCLOG(LOG_LEVEL_WARNING) << "Version message checksum error: expect "
                                   << header.checksum() << ", received "
                                   << version->GetHash().GetLow32();
            return nullptr;
        }
        msg = version;
    }
    else if (header.command() == msg_command::kMsgVerack) {
        Verack *verack = new Verack();
        verack->Deserialize(byte_source);
        if (header.checksum() != verack->GetHash().GetLow32()) {
            BTCLOG(LOG_LEVEL_WARNING) << "Verack message checksum error: expect "
                                   << header.checksum() << ", received "
                                   << verack->GetHash().GetLow32();
            return nullptr;
        }
        msg = verack;
    }
    else if (header.command() == msg_command::kMsgAddr) {
        Addr *addr = new Addr();
        addr->Deserialize(byte_source);
        if (header.checksum() != addr->GetHash().GetLow32()) {
            BTCLOG(LOG_LEVEL_WARNING) << "Addr message checksum error: expect "
                                   << header.checksum() << ", received "
                                   << addr->GetHash().GetLow32();
            return nullptr;
        }
        msg = addr;
    }
    else if (header.command() == msg_command::kMsgInv) {
        Inv *inv = new Inv();
        inv->Deserialize(byte_source);
        if (header.checksum() != inv->GetHash().GetLow32()) {
            BTCLOG(LOG_LEVEL_WARNING) << "Inv message checksum error: expect "
                                   << header.checksum() << ", received "
                                   << inv->GetHash().GetLow32();
            return nullptr;
        }
        msg = inv;
    }
    else if (header.command() == msg_command::kMsgGetAddr) {
        GetAddr *getaddr = new GetAddr();
        getaddr->Deserialize(byte_source);
        if (header.checksum() != getaddr->GetHash().GetLow32()) {
            BTCLOG(LOG_LEVEL_WARNING) << "Getaddr message checksum error: expect "
                                   << header.checksum() << ", received "
                                   << getaddr->GetHash().GetLow32();
            return nullptr;
        }
        msg = getaddr;
    }
    else if (header.command() == msg_command::kMsgPing) {
        Ping *ping = new Ping();
        ping->set_protocol_version(protocol_version);
        ping->Deserialize(byte_source);
        if (header.checksum() != ping->GetHash().GetLow32()) {
            BTCLOG(LOG_LEVEL_WARNING) << "Ping message checksum error: expect "
                                   << header.checksum() << ", received "
                                   << ping->GetHash().GetLow32();
            return nullptr;
        }
        msg = ping;
    }
    else if (header.command() == msg_command::kMsgPong) {
        Pong *pong = new Pong();
        pong->Deserialize(byte_source);
        if (header.checksum() != pong->GetHash().GetLow32()) {
            BTCLOG(LOG_LEVEL_WARNING) << "Pong message checksum error: expect "
                                   << header.checksum() << ", received "
                                   << pong->GetHash().GetLow32();
            return nullptr;
        }
        msg = pong;
    }
    else if (header.command() == msg_command::kMsgReject) {
        Reject *reject = new Reject();
        reject->Deserialize(byte_source);
        if (header.checksum() != reject->GetHash().GetLow32()) {
            BTCLOG(LOG_LEVEL_WARNING) << "Reject message checksum error: expect "
                                   << header.checksum() << ", received "
                                   << reject->GetHash().GetLow32();
            return nullptr;
        }
        msg = reject;
    }
    else if (header.command() == msg_command::kMsgSendHeaders) {
        SendHeaders *send_headers = new SendHeaders();
        send_headers->Deserialize(byte_source);
        if (header.checksum() != send_headers->GetHash().GetLow32()) {
            BTCLOG(LOG_LEVEL_WARNING) << "Sendheaders message checksum error: expect "
                                   << header.checksum() << ", received "
                                   << send_headers->GetHash().GetLow32();
            return nullptr;
        }
        msg = send_headers;
    }
    else if (header.command() == msg_command::kMsgSendCmpct) {
        SendCmpct *send_compact = new SendCmpct();
        send_compact->Deserialize(byte_source);
        if (header.checksum() != send_compact->GetHash().GetLow32()) {
            BTCLOG(LOG_LEVEL_WARNING) << "Sendheaders message checksum error: expect "
                                   << header.checksum() << ", received "
                                   << send_compact->GetHash().GetLow32();
            return nullptr;
        }
        msg = send_compact;
    }
    else {
        BTCLOG(LOG_LEVEL_WARNING) << "Rececived unknown message: "
                                  << header.command();
        return nullptr;
    }
    
    if (!msg->IsValid()) {
        BTCLOG(LOG_LEVEL_ERROR) << "Received invalid " << header.command() 
                                << " message data";
        delete msg;
        return nullptr;
    }
    
    return msg;
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
        MessageData *message = MsgDataFactory(raw, header, src_node->protocol().version());
        evbuffer_drain(buf, header.payload_length());
        if (message) {
            if (CheckMisbehaving(header.command(), src_node)) {
                ret &= message->RecvHandler(src_node, params);
                delete message;
            }
            else {
                BTCLOG(LOG_LEVEL_ERROR) << "Received misbehavior message from peer "
                                        << src_node->id();
                src_node->mutable_misbehavior()->Misbehaving(src_node->id(), 1);
                ret = false;
            }
        }
        else {
            BTCLOG(LOG_LEVEL_ERROR) << "Received invalid message from peer "
                                    << src_node->id();
            ret = false;
        }        
      
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
