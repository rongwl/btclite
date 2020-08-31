#ifndef BTCLITE_PROTOCOL_REJECT_H
#define BTCLITE_PROTOCOL_REJECT_H


#include <cstdint>

#include "arithmetic.h"
#include "message.h"


namespace btclite {
namespace network {
namespace protocol {

// "reject" message codes
enum class CCode : uint8_t {
    kRejectUnknown = 0,
    kRejectMalformed = 0x01,
    kRejectInvalid = 0x10,
    kRejectObsolete = 0x11,
    kRejectDuplicate = 0x12,
    kRejectNonstandard = 0x40,
    kRejectDust = 0x41, // part of bip 61
    kRejectInsufficientfee = 0x42,
    kRejectCheckpoint = 0x43
};

class Reject {
public:
    Reject() = default;
    
    Reject(const std::string& message, CCode ccode, const std::string& reason);
    Reject(std::string&& message, CCode ccode, std::string&& reason) noexcept;
    
    Reject(const std::string& message, CCode ccode, const std::string& reason,
           const util::Hash256& data);    
    Reject(std::string&& message, CCode ccode, std::string&& reason,
           const util::Hash256& data) noexcept;
    
    //-------------------------------------------------------------------------
    bool RecvHandler(std::shared_ptr<Node> src_node) const;    
    std::string Command() const;    
    bool IsValid() const;
    void Clear();
    size_t SerializedSize() const;    
    util::Hash256 GetHash() const;
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& out) const;
    template <typename Stream>
    void Deserialize(Stream& in);
    
    //-------------------------------------------------------------------------
    bool operator==(const Reject& b) const;    
    bool operator!=(const Reject& b) const;
    
    //-------------------------------------------------------------------------
    const std::string& message() const;
    void set_message(const std::string& message);
    void set_message(std::string&& message) noexcept;
    
    CCode ccode() const;
    void set_ccode(CCode ccode);
    
    const std::string& reason() const;
    void set_reason(const std::string& reason);
    void set_reason(std::string&& reason) noexcept;
    
    const util::Hash256& data() const;
    void set_data(const util::Hash256& data);  
    
private:
    std::string message_;
    CCode ccode_ = CCode::kRejectUnknown;
    std::string reason_;
    util::Hash256 data_;
};

template <typename Stream>
void Reject::Serialize(Stream& out) const
{
    util::Serializer<Stream> serializer(out);
    
    if (message_.size() > MessageHeader::kCommandSize)
        serializer.SerialWrite(message_.substr(0, MessageHeader::kCommandSize));
    else
        serializer.SerialWrite(message_);
    
    serializer.SerialWrite(ccode_);
    
    if (reason_.size() > kMaxRejectMessageLength)
        serializer.SerialWrite(reason_.substr(0, kMaxRejectMessageLength));
    else
        serializer.SerialWrite(reason_);
    
    if (message_ == msg_command::kMsgBlock || message_ == msg_command::kMsgTx)
        serializer.SerialWrite(data_);
}

template <typename Stream>
void Reject::Deserialize(Stream& in)
{
    util::Deserializer<Stream> deserializer(in);
    
    deserializer.SerialRead(&message_);
    if (message_.size() > MessageHeader::kCommandSize)
        message_.resize(MessageHeader::kCommandSize);
    
    deserializer.SerialRead(&ccode_);
    
    deserializer.SerialRead(&reason_);    
    if (reason_.size() > kMaxRejectMessageLength)
        reason_.resize(kMaxRejectMessageLength);
    
    if (message_ == msg_command::kMsgBlock || message_ == msg_command::kMsgTx)
        deserializer.SerialRead(&data_);
}

} // namespace protocol
} // namespace network
} // namespace btclite

#endif // BTCLITE_PROTOCOL_REJECT_H
