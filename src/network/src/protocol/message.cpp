#include "protocol/message.h"

#include "network/include/params.h"


namespace btclite {
namespace network {
namespace protocol {

MessageHeader::MessageHeader(const uint8_t *raw_data)
{
    std::vector<uint8_t> vec;
    util::ByteSource<std::vector<uint8_t> > byte_source(vec);
    
    std::memset(command_.begin(), 0, kCommandSize);
    vec.reserve(kSize);
    vec.assign(raw_data, raw_data + kSize);
    Deserialize(byte_source);
}

bool MessageHeader::IsValid(uint32_t magic) const
{
    if (magic == kMainMagic || magic == kTestnetMagic || magic == kRegtestMagic) {
        if (magic_ != magic) {
            BTCLOG(LOG_LEVEL_WARNING) << "MessageHeader::magic_(" << magic_ 
                                      << ") is invalid, " << magic << " is correct";
            return false;
        }
    }
    else {
        if (magic_ != kMainMagic && magic_ != kTestnetMagic && magic_ != kRegtestMagic) {
            BTCLOG(LOG_LEVEL_WARNING) << "MessageHeader::magic_(" << magic_ 
                                      << ") is invalid";
            return false;
        }
    }
    
    std::string cmd = command();
    if (cmd != msg_command::kMsgVersion &&
            cmd != msg_command::kMsgVerack &&
            cmd != msg_command::kMsgAddr &&
            cmd != msg_command::kMsgInv &&
            cmd != msg_command::kMsgGetData &&
            cmd != msg_command::kMsgMerkleBlock &&
            cmd != msg_command::kMsgGetBlocks &&
            cmd != msg_command::kMsgGetHeaders &&
            cmd != msg_command::kMsgTx &&
            cmd != msg_command::kMsgHeaders &&
            cmd != msg_command::kMsgBlock &&
            cmd != msg_command::kMsgGetAddr &&
            cmd != msg_command::kMsgMempool &&
            cmd != msg_command::kMsgPing &&
            cmd != msg_command::kMsgPong &&
            cmd != msg_command::kMsgNotFound &&
            cmd != msg_command::kMsgFilterLoad &&
            cmd != msg_command::kMsgFilterAdd &&
            cmd != msg_command::kMsgFilterClear &&
            cmd != msg_command::kMsgReject &&
            cmd != msg_command::kMsgSendHeaders &&
            cmd != msg_command::kMsgFeeFilter &&
            cmd != msg_command::kMsgSendCmpct &&
            cmd != msg_command::kMsgCmpctBlock &&
            cmd != msg_command::kMsgGetBlockTxn &&
            cmd != msg_command::kMsgBlockTxn) {
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
    if (command != msg_command::kMsgVersion && src_node->protocol().version() == 0)
        return false;
    
    // Must have a verack message before anything else
    if (command != msg_command::kMsgVersion && command != msg_command::kMsgVerack &&
            src_node->connection().connection_state() != NodeConnection::kEstablished)
        return false;
    
    return true;
}

} // namespace protocol
} // namespace network
} // namespace btclite
