#ifndef BTCLITE_MSG_PROCESS_H
#define BTCLITE_MSG_PROCESS_H

#include "hash.h"
#include "network/include/params.h"
#include "protocol/version.h"


namespace btclite {
namespace network {
namespace msgprocess{

MessageData *MsgDataFactory(const MessageHeader& header, const uint8_t *raw);

bool ParseMsg(std::shared_ptr<Node> src_node);

template <typename Message>
bool SendMsg(const Message& msg, std::shared_ptr<Node> dst_node)
{
    MessageHeader header(btclite::network::SingletonParams::GetInstance().msg_magic());
    std::vector<uint8_t> vec_msg, vec_header, vec;
    ByteSink<std::vector<uint8_t> > byte_sink_msg(vec_msg), byte_sink_header(vec_header), byte_sink(vec);
    Hash256 hash256;
    
    if (!dst_node->bev())
        return false;
    
    msg.GetHash(&hash256);
    header.set_command(msg.kCommand);
    header.set_payload_length(vec_msg.size());
    header.set_checksum(hash256.GetLow64());
    header.Serialize(byte_sink);
    
    if (vec.size() != MessageHeader::kSize) {
        BTCLOG(LOG_LEVEL_ERROR) << "Wrong message header size:" << vec_header.size()
                                << ", message type:" << msg.kCommand;
        return false;
    }
    
    msg.Serialize(byte_sink);
    
    if (!dst_node->BevThreadSafeWrite(vec)) {
        BTCLOG(LOG_LEVEL_ERROR) << "Writing message to bufferevent failed, peer:" << dst_node->id();
        return false;
    }
    
    return true;
}

} // namespace msgprocess
} // namespace network
} // namespace btclite

#endif // BTCLITE_MSG_PROCESS_H
