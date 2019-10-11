#ifndef BTCLITE_MSG_PROCESS_H
#define BTCLITE_MSG_PROCESS_H

#include "hash.h"
#include "network/include/params.h"
#include "protocol/message.h"
#include "stream.h"


namespace btclite {
namespace network {
namespace msgprocess{

MessageData *MsgDataFactory(const MessageHeader& header, const uint8_t *raw);
bool ParseMsg(std::shared_ptr<Node> src_node);

template <typename Message>
bool SendMsg(const Message& msg, std::shared_ptr<Node> dst_node)
{
    MessageHeader header(btclite::network::SingletonParams::GetInstance().msg_magic());
    Hash256 hash256;
    MemOstream ms;
    HashOStream hs;
    
    if (!dst_node->bev())
        return false;
    
    hs << msg;
    hs.Sha256(&hash256);
    header.set_command(msg.kCommand);
    header.set_payload_length(hs.vec().size());
    header.set_checksum(hash256.GetLow64());
    ms << header;
    
    if (ms.vec().size() != MessageHeader::kSize) {
        BTCLOG(LOG_LEVEL_ERROR) << "Wrong message header size:" << ms.vec().size()
                                << ", message type:" << msg.kCommand;
        return false;
    }
    
    bufferevent_lock(dst_node->mutable_bev());
    
    if (bufferevent_write(dst_node->mutable_bev(), ms.vec().data(), ms.vec().size())) {
        BTCLOG(LOG_LEVEL_ERROR) << "Writing message header to bufferevent failed, peer:" << dst_node->id();
        return false;
    }
    
    if (bufferevent_write(dst_node->mutable_bev(), hs.vec().data(), hs.vec().size())) {
        BTCLOG(LOG_LEVEL_ERROR) << "Writing message data to bufferevent failed, peer:" << dst_node->id();
        return false;
    }
    
    bufferevent_unlock(dst_node->mutable_bev());
    
    return true;
}

bool SendVerMsg(std::shared_ptr<Node> dst_node);

} // namespace msgprocess
} // namespace network
} // namespace btclite

#endif // BTCLITE_MSG_PROCESS_H
