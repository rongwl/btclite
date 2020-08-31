#include "protocol/message.h"

#include "network/include/params.h"


namespace btclite {
namespace network {
namespace protocol {

MessageHeader::MessageHeader(uint32_t magic, const std::string& command,
              uint32_t payload_length, uint32_t checksum)
    : magic_(magic), payload_length_(payload_length),
      checksum_(checksum)
{
    set_command(command);
}

MessageHeader::MessageHeader(uint32_t magic, std::string&& command,
              uint32_t payload_length, uint32_t checksum) noexcept
    : magic_(magic), payload_length_(payload_length),
      checksum_(checksum)
{
    set_command(std::move(command));
}

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
    if (magic == kMainnetMagic || magic == kTestnetMagic || magic == kRegtestMagic) {
        if (magic_ != magic) {
            BTCLOG(LOG_LEVEL_WARNING) << "MessageHeader::magic_(" << magic_ 
                                      << ") is invalid, " << magic << " is correct";
            return false;
        }
    }
    else {
        if (magic_ != kMainnetMagic && magic_ != kTestnetMagic && magic_ != kRegtestMagic) {
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
        BTCLOG(LOG_LEVEL_WARNING) << "MessageHeader::payload_length_(" << payload_length_ << ") is oversize";
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

bool MessageHeader::operator==(const MessageHeader& b) const
{
    return (magic_ == b.magic_) &&
           (command_ == b.command_) &&
           (payload_length_ == b.payload_length_) &&
           (checksum_ == b.checksum_);
}

bool MessageHeader::operator!=(const MessageHeader& b) const
{
    return !(*this == b);
}

uint32_t MessageHeader::magic() const
{
    return magic_;
}

void MessageHeader::set_magic(uint32_t magic)
{
    magic_ = magic;
}

std::string MessageHeader::command() const
{
    const char *end = (const char*)std::memchr(command_.begin(), '\0', kCommandSize);
    size_t size = end ? (end - command_.begin()) : kCommandSize;
    return std::string(command_.begin(), size);
}

void MessageHeader::set_command(const std::string& command)
{
    std::memset(command_.begin(), 0, kCommandSize);
    size_t size = command.size() < kCommandSize ? command.size() : kCommandSize;
    std::memcpy(command_.begin(), command.data(), size);
}

void MessageHeader::set_command(std::string&& command) noexcept
{
    std::memset(command_.begin(), 0, kCommandSize);
    size_t size = command.size() < kCommandSize ? command.size() : kCommandSize;
    std::memmove(command_.begin(), command.data(), size);
}

uint32_t MessageHeader::payload_length() const
{
    return payload_length_;
}

void MessageHeader::set_payload_length(uint32_t payload_length)
{
    payload_length_ = payload_length;
}

uint32_t MessageHeader::checksum() const
{
    return checksum_;
}

void MessageHeader::set_checksum(uint32_t checksum)
{
    checksum_ = checksum;
}

bool CheckMisbehaving(const std::string command, std::shared_ptr<Node> src_node)
{
    // Must have a version message before anything else
    if (command != msg_command::kMsgVersion && src_node->protocol().version == 0)
        return false;
    
    // Must have a verack message before anything else
    if (command != msg_command::kMsgVersion && command != msg_command::kMsgVerack &&
            !src_node->connection().IsHandshakeCompleted())
        return false;
    
    return true;
}

} // namespace protocol
} // namespace network
} // namespace btclite
