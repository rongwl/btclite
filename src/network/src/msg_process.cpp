#include "msg_process.h"

#include <event2/buffer.h>

#include "protocol/address.h"
#include "protocol/ping.h"
#include "protocol/reject.h"
#include "protocol/verack.h"
#include "protocol/version.h"


using namespace btclite::network::protocol;

namespace btclite {
namespace network {
namespace msg_process{

MessageData *MsgDataFactory(const MessageHeader& header, const uint8_t *raw)
{    
    std::vector<uint8_t> vec;
    ByteSource<std::vector<uint8_t> > byte_source(vec);
    MessageData *msg = nullptr;
    
    if (!header.IsValid())
        return nullptr;
    
    if (!raw && header.command() != kMsgVerack)
        return nullptr;        
    
    vec.reserve(header.payload_length());
    vec.assign(raw, raw + header.payload_length());
    
    if (header.command() == kMsgVersion) {        
        Version *version = new Version();
        version->Deserialize(byte_source);
        msg = version;
    }
    else if (header.command() == kMsgVerack) {
        Verack *verack = new Verack();
        verack->Deserialize(byte_source);
        msg = verack;
    }
    else if (header.command() == kMsgAddr) {
        Addr *addr = new Addr();
        addr->Deserialize(byte_source);
        msg = addr;
    }
    else if (header.command() == kMsgPing) {
        Ping *ping = new Ping();
        ping->Deserialize(byte_source);
        msg = ping;
    }
    else if (header.command() == kMsgReject) {
        Reject *reject = new Reject();
        reject->Deserialize(byte_source);
        msg = reject;
    }
    else
        return nullptr;
    
    if (!msg->IsValid()) {
        BTCLOG(LOG_LEVEL_ERROR) << "Received invalid " << header.command() << " message data.";
        delete msg;
        return nullptr;
    }
    
    return msg;
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
        MessageHeader header;
        if (!header.Init(raw)) {
            BTCLOG(LOG_LEVEL_ERROR) << "Received invalid message header from peer " 
                                    << src_node->id();
            return false;
        }
        evbuffer_drain(buf, MessageHeader::kSize);
        
        raw = evbuffer_pullup(buf, header.payload_length());
        MessageData *message = MsgDataFactory(header, raw);
        if (!message) {
            BTCLOG(LOG_LEVEL_ERROR) << "Prasing message data from peer "
                                    << src_node->id() << " failed.";
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

bool SendVersion(std::shared_ptr<Node> dst_node)
{
    ServiceFlags services = dst_node->services();
    uint32_t start_height = SingletonBlockChain::GetInstance().Height();
    btclite::network::NetAddr addr_recv(dst_node->addr());
    btclite::network::NetAddr addr_from;
    
    addr_from.mutable_proto_addr()->set_services(services);
    Version ver_msg(kProtocolVersion, services,
                    btclite::utility::util_time::GetTimeSeconds(),
                    std::move(addr_recv), std::move(addr_from),
                    dst_node->local_host_nonce(), std::move(FormatUserAgent()),
                    start_height, true);

    if (!SendMsg(ver_msg, dst_node))
        return false;

    BTCLOG(LOG_LEVEL_INFO) << "Send version message: version " << kProtocolVersion 
                           << ", start_height=" << start_height << ", addr_recv=" << addr_recv.ToString() 
                           << ", peer=" << dst_node->id();
    
    return true;
}

bool SendAddr(std::shared_ptr<Node> dst_node)
{
    return true;
}

bool SendRejects(std::shared_ptr<Node> dst_node)
{
    BlockSync &block_sync = SingletonBlockSync::GetInstance();
    std::vector<BlockReject> rejects;
    
    if (!block_sync.GetRejects(dst_node->id(), &rejects))
        return false;
    
    for (auto it = rejects.begin(); it != rejects.end(); ++it) {
        Reject reject;
        reject.Clear();
        reject.set_message(std::move(std::string(kMsgBlock)));
        reject.set_ccode(it->reject_code);
        reject.set_reason(std::move(it->reject_reason));
        reject.set_data(std::move(it->block_hash));
        SendMsg(reject, dst_node);
    }
    block_sync.ClearRejects(dst_node->id());
    
    return true;
}

void UpdatePreferredDownload(std::shared_ptr<Node> node)
{
    BlockSync& block_sync = SingletonBlockSync::GetInstance();
    bool old_state;
    bool new_state = (!node->is_inbound() && !node->IsClient());
    
    if (!block_sync.GetPreferredDownload(node->id(), &old_state))
        return;
    
    block_sync.SetPreferredDownload(node->id(), new_state);
    if (!old_state && new_state)
        block_sync.set_preferred_download_count(block_sync.preferred_download_count()+1);
    else if (old_state && !new_state)
        block_sync.set_preferred_download_count(block_sync.preferred_download_count()-1);
}

} // namespace msg_process
} // namespace network
} // namespace btclite
