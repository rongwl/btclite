#ifndef BTCLITE_PROTOCOL_REJECT_H
#define BTCLITE_PROTOCOL_REJECT_H


#include <cstdint>

#include "hash.h"
#include "message.h"


namespace btclite {
namespace network {
namespace protocol {

// "reject" message codes
enum CCode : uint8_t {
    kRejectMalformed = 0x01,
    kRejectInvalid = 0x10,
    kRejectObsolete = 0x11,
    kRejectDuplicate = 0x12,
    kRejectNonstandard = 0x40,
    kRejectDust = 0x41, // part of bip 61
    kRejectInsufficientfee = 0x42,
    kRejectCheckpoint = 0x43
};

class Reject : public MessageData {
public:
    static const std::string kCommand;
    
    //-------------------------------------------------------------------------
    bool RecvHandler(std::shared_ptr<Node> src_node) const;
    bool IsValid() const;
    void Clear();
    size_t SerializedSize() const;
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& out) const;
    template <typename Stream>
    void Deserialize(Stream& in);
    
    //-------------------------------------------------------------------------
    bool operator==(const Reject& b) const;    
    bool operator!=(const Reject& b) const;
    
    //-------------------------------------------------------------------------
    const std::string& message() const
    {
        return message_;
    }
    void set_message(const std::string& message)
    {
        message_ = message;
    }
    void set_message(std::string&& message) noexcept
    {
        message_ = std::move(message);
    }
    
    uint8_t ccode() const
    {
        return ccode_;
    }
    void set_ccode(uint8_t ccode)
    {
        ccode_ = ccode;
    }
    
    const std::string& reason() const
    {
        return reason_;
    }
    void set_reason(const std::string& reason)
    {
        reason_ = reason;
    }
    void set_reason(std::string&& reason) noexcept
    {
        reason_ = std::move(reason);
    }
    
    const Hash256& data() const
    {
        return data_;
    }
    void set_data(const Hash256& data)
    {
        data_ = data;
    }
    void set_data(Hash256&& data) noexcept
    {
        data_ = std::move(data);
    }    
    
private:
    std::string message_;
    uint8_t ccode_ = 0;
    std::string reason_;
    Hash256 data_;
};

template <typename Stream>
void Reject::Serialize(Stream& out) const
{
    Serializer<Stream> serializer(out);
    
    if (message_.size() > MessageHeader::kCommandSize)
        serializer.SerialWrite(message_.substr(0, MessageHeader::kCommandSize));
    else
        serializer.SerialWrite(message_);
    serializer.SerialWrite(ccode_);
    if (reason_.size() > kMaxRejectMessageLength)
        serializer.SerialWrite(reason_.substr(0, kMaxRejectMessageLength));
    else
        serializer.SerialWrite(reason_);
    serializer.SerialWrite(data_);
}

template <typename Stream>
void Reject::Deserialize(Stream& in)
{
    Deserializer<Stream> deserializer(in);
    
    deserializer.SerialRead(&message_);
    if (message_.size() > MessageHeader::kCommandSize)
        message_.resize(MessageHeader::kCommandSize);
    deserializer.SerialRead(&ccode_);
    deserializer.SerialRead(&reason_);
    if (reason_.size() > kMaxRejectMessageLength)
        reason_.resize(kMaxRejectMessageLength);
    deserializer.SerialRead(&data_);
}

} // namespace protocol
} // namespace network
} // namespace btclite

#endif // BTCLITE_PROTOCOL_REJECT_H
