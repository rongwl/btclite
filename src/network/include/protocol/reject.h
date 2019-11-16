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

class reject : public MessageData {
public:
    reject() = default;
    
    reject(const std::string& message, uint8_t ccode, const std::string& reason)
        : message_(message), ccode_(ccode), reason_(reason), data_() {}
    
    reject(std::string&& message, uint8_t ccode, std::string&& reason) noexcept
        : message_(std::move(message)), ccode_(ccode), reason_(std::move(reason)),
          data_() {}
    
    reject(const std::string& message, uint8_t ccode, const std::string& reason,
           const Hash256& data)
        : message_(message), ccode_(ccode), reason_(reason), data_(data) {}
    
    reject(std::string&& message, uint8_t ccode, std::string&& reason,
           const Hash256& data) noexcept
        : message_(std::move(message)), ccode_(ccode), reason_(std::move(reason)),
          data_(data) {}
    
    //-------------------------------------------------------------------------
    bool RecvHandler(std::shared_ptr<Node> src_node) const;
    
    std::string Command() const
    {
        return kMsgReject;
    }
    
    bool IsValid() const;
    void Clear();
    size_t SerializedSize() const;
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& out) const;
    template <typename Stream>
    void Deserialize(Stream& in);
    
    //-------------------------------------------------------------------------
    bool operator==(const reject& b) const;    
    bool operator!=(const reject& b) const;
    
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
    
private:
    std::string message_;
    uint8_t ccode_ = 0;
    std::string reason_;
    Hash256 data_;
};

template <typename Stream>
void reject::Serialize(Stream& out) const
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
    
    if (message_ == ::kMsgBlock || message_ == ::kMsgTx)
        serializer.SerialWrite(data_);
}

template <typename Stream>
void reject::Deserialize(Stream& in)
{
    Deserializer<Stream> deserializer(in);
    
    deserializer.SerialRead(&message_);
    if (message_.size() > MessageHeader::kCommandSize)
        message_.resize(MessageHeader::kCommandSize);
    
    deserializer.SerialRead(&ccode_);
    
    deserializer.SerialRead(&reason_);    
    if (reason_.size() > kMaxRejectMessageLength)
        reason_.resize(kMaxRejectMessageLength);
    
    if (message_ == ::kMsgBlock || message_ == ::kMsgTx)
        deserializer.SerialRead(&data_);
}

using Reject = Hashable<reject>;

} // namespace protocol
} // namespace network
} // namespace btclite

#endif // BTCLITE_PROTOCOL_REJECT_H
