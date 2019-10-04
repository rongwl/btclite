#ifndef BTCLITE_MESSAGE_TYPES_MESSAGE_H
#define BTCLITE_MESSAGE_TYPES_MESSAGE_H


#include "node.h"


class MessageHeader {
public:
    static constexpr size_t kMessageStartSize = 4;
    static constexpr size_t kCommandSize = 12;
    static constexpr size_t kPayloadSize = 4;
    static constexpr size_t kChecksumSize = 4;
    static constexpr size_t kSize = kMessageStartSize + kCommandSize + kPayloadSize + kChecksumSize;
    
    using MsgMagic = uint32_t;
    
    //-------------------------------------------------------------------------
    MessageHeader()
        : magic_(0), command_(), payload_length_(0), checksum_(0)
    {
        std::memset(command_.begin(), 0, kCommandSize);
    }
    
    explicit MessageHeader(uint32_t magic)
        : magic_(magic), command_(), payload_length_(0), checksum_(0) 
    {
        std::memset(command_.begin(), 0, kCommandSize);
    }
    
    explicit MessageHeader(const uint8_t *raw_data);
    
    MessageHeader(uint32_t magic, const std::string& command,
                  uint32_t payload_length, uint32_t checksum)
        : magic_(magic), command_(), payload_length_(payload_length), checksum_(checksum) 
    {
        std::memset(command_.begin(), 0, kCommandSize);
        set_command(command);
    }
    
    MessageHeader(const MessageHeader& header)
        : magic_(header.magic_), command_(header.command_),
          payload_length_(header.payload_length_), checksum_(header.checksum_) {}
    
    MessageHeader(MessageHeader&& header) noexcept
        : magic_(header.magic_), command_(std::move(header.command_)),
          payload_length_(header.payload_length_), checksum_(header.checksum_) {}
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& out) const;
    template <typename Stream>
    void Deserialize(Stream& in);
    
    //-------------------------------------------------------------------------
    bool IsValid() const;
    
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
    uint32_t magic_;
    std::array<char, kCommandSize> command_;
    uint32_t payload_length_;
    uint32_t checksum_;
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
};

// mixin GetHash()for MessageData classes
template <typename Base>
struct MsgHashable : public Base {
    using Base::Base;
    
    void GetHash(Hash256 *hash) const;
};

template <typename Base>
void MsgHashable<Base>::GetHash(Hash256 *hash) const
{
    std::vector<uint8_t> vec;
    ByteSink<std::vector<uint8_t> > byte_sink(vec);
    
    Base::Serialize(byte_sink);
    btclite::crypto::hash::Sha256(vec, hash);
}


#endif // BTCLITE_MESSAGE_TYPES_MESSAGE_H
