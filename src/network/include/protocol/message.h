#ifndef BTCLITE_PROTOCOL_MESSAGE_H
#define BTCLITE_PROTOCOL_MESSAGE_H


#include "hash.h"
#include "node.h"


namespace btclite {
namespace network {
namespace protocol {

class MessageHeader {
public:
    static constexpr size_t kMessageStartSize = 4;
    static constexpr size_t kCommandSize = 12;
    static constexpr size_t kPayloadSize = 4;
    static constexpr size_t kChecksumSize = 4;
    static constexpr size_t kSize = kMessageStartSize + kCommandSize
                                    + kPayloadSize + kChecksumSize;
    
    using MsgMagic = uint32_t;
    
    MessageHeader() = default;
    
    MessageHeader(uint32_t magic, const std::string& command,
                  uint32_t payload_length, uint32_t checksum)
        : magic_(magic), payload_length_(payload_length),
          checksum_(checksum)
    {
        set_command(command);
    }
    
    MessageHeader(uint32_t magic, std::string&& command,
                  uint32_t payload_length, uint32_t checksum) noexcept
        : magic_(magic), payload_length_(payload_length),
          checksum_(checksum)
    {
        set_command(std::move(command));
    }
    
    explicit MessageHeader(const uint8_t *raw);
    
    //-------------------------------------------------------------------------
    bool IsValid() const;
    void Clear();
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& out) const;
    template <typename Stream>
    void Deserialize(Stream& in);
    
    //-------------------------------------------------------------------------
    bool operator==(const MessageHeader& b) const
    {
        return (magic_ == b.magic_) &&
               (command_ == b.command_) &&
               (payload_length_ == b.payload_length_) &&
               (checksum_ == b.checksum_);
    }
    
    bool operator!=(const MessageHeader& b) const
    {
        return !(*this == b);
    }
    
    //-------------------------------------------------------------------------
    uint32_t magic() const
    {
        return magic_;
    }
    void set_magic(uint32_t magic)
    {
        magic_ = magic;
    }

    std::string command() const
    {
        const char *end = (const char*)std::memchr(command_.begin(), '\0', kCommandSize);
        size_t size = end ? (end - command_.begin()) : kCommandSize;
        return std::string(command_.begin(), size);
    }
    
    void set_command(const std::string& command)
    {
        std::memset(command_.begin(), 0, kCommandSize);
        size_t size = command.size() < kCommandSize ? command.size() : kCommandSize;
        std::memcpy(command_.begin(), command.data(), size);
    }
    void set_command(std::string&& command) noexcept
    {
        std::memset(command_.begin(), 0, kCommandSize);
        size_t size = command.size() < kCommandSize ? command.size() : kCommandSize;
        std::memmove(command_.begin(), command.data(), size);
    }

    uint32_t payload_length() const
    {
        return payload_length_;
    }
    void set_payload_length(uint32_t payload_length)
    {
        payload_length_ = payload_length;
    }

    uint32_t checksum() const
    {
        return checksum_;
    }
    void set_checksum(uint32_t checksum)
    {
        checksum_ = checksum;
    }
    
private:    
    uint32_t magic_ = 0;
    std::array<char, kCommandSize> command_ = {};
    uint32_t payload_length_ = 0;
    uint32_t checksum_ = 0;
};

template <typename Stream>
void MessageHeader::Serialize(Stream& out) const
{
    Serializer<Stream> serializer(out);
    serializer.SerialWrite(magic_);
    serializer.SerialWrite(command_);
    serializer.SerialWrite(payload_length_);
    serializer.SerialWrite(checksum_);
}

template <typename Stream>
void MessageHeader::Deserialize(Stream& in)
{
    Deserializer<Stream> deserializer(in);
    deserializer.SerialRead(&magic_);
    deserializer.SerialRead(&command_);
    deserializer.SerialRead(&payload_length_);
    deserializer.SerialRead(&checksum_);
}

class MessageData {
public:
    virtual bool RecvHandler(std::shared_ptr<Node> src_node) const = 0;
    virtual std::string Command() const = 0;
    virtual size_t SerializedSize() const = 0;
    virtual bool IsValid() const = 0;
    virtual ~MessageData() {}
};

bool CheckMisbehaving(const std::string command, std::shared_ptr<Node> src_node);

} // namespace protocol
} // namespace network
} // namespace btclite


#endif // BTCLITE_MESSAGE_TYPES_MESSAGE_H
