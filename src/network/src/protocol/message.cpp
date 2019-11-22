#include "protocol/message.h"

#include "network/include/params.h"


namespace btclite {
namespace network {
namespace protocol {

MessageHeader::MessageHeader(const uint8_t *raw_data)
{
    std::vector<uint8_t> vec;
    ByteSource<std::vector<uint8_t> > byte_source(vec);
    
    std::memset(command_.begin(), 0, kCommandSize);
    vec.reserve(kSize);
    vec.assign(raw_data, raw_data + kSize);
    Deserialize(byte_source);
}

bool MessageHeader::IsValid() const
{
    if (magic_ != btclite::network::SingletonParams::GetInstance().msg_magic()) {
        BTCLOG(LOG_LEVEL_WARNING) << "MessageHeader::magic_(" << magic_ << ") is invalid";
        return false;
    }
    
    std::string cmd = command();
    if (cmd != kMsgVersion &&
            cmd != kMsgVerack &&
            cmd != kMsgAddr &&
            cmd != kMsgInv &&
            cmd != kMsgGetData &&
            cmd != kMsgMerkleBlock &&
            cmd != kMsgGetBlocks &&
            cmd != kMsgGetHeaders &&
            cmd != ::kMsgTx &&
            cmd != kMsgHeaders &&
            cmd != ::kMsgBlock &&
            cmd != kMsgGetAddr &&
            cmd != kMsgMempool &&
            cmd != kMsgPing &&
            cmd != kMsgPong &&
            cmd != kMsgNotFound &&
            cmd != kMsgFilterLoad &&
            cmd != kMsgFilterAdd &&
            cmd != kMsgFilterClear &&
            cmd != kMsgReject &&
            cmd != kMsgSendHeaders &&
            cmd != kMsgFeeFilter &&
            cmd != kMsgSendCmpct &&
            cmd != kMsgCmpctBlock &&
            cmd != kMsgGetBlockTxn &&
            cmd != kMsgBlockTxn) {
        BTCLOG(LOG_LEVEL_WARNING) << "MessageHeader::command_(" << cmd << ") is invalid";
        return false;
    }
    
    if (payload_length_ > kMaxMessageSize) {
        BTCLOG(LOG_LEVEL_WARNING) << "MessageHeader::payload_length_(" << payload_length_ << ") is invalid";
        return false;
    }
    
    return true;
}

void MessageHeader::Clear()
{
    magic_ = 0;
    std::memset(command_.begin(), 0, kCommandSize);
    payload_length_ = 0;
    checksum_ = 0;
}

bool CheckMisbehaving(const std::string command, std::shared_ptr<Node> src_node)
{
    // Must have a version message before anything else
    if (command != kMsgVersion && src_node->protocol_version() == 0)
        return false;
    
    // Must have a verack message before anything else
    if (command != kMsgVersion && command != kMsgVerack &&
            !src_node->conn_established())
        return false;
    
    return true;
}

} // namespace protocol
} // namespace network
} // namespace btclite
