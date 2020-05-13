#ifndef BTCLITE_PROTOCOL_SEND_COMPACT_H
#define BTCLITE_PROTOCOL_SEND_COMPACT_H


#include "message.h"


namespace btclite {
namespace network {
namespace protocol {

class SendCmpct {
public:
    SendCmpct() = default;
    
    SendCmpct(bool high_bandwidth_mode, uint64_t version)
        : high_bandwidth_mode_(high_bandwidth_mode), version_(version) {}
    
    //-------------------------------------------------------------------------
    bool RecvHandler(std::shared_ptr<Node> src_node) const;
    
    std::string Command() const
    {
        return msg_command::kMsgSendCmpct;
    }
    
    bool IsValid() const
    {
        return (version_ == 1 || version_ == 2);
    }
    
    void Clear() 
    {
        high_bandwidth_mode_ = false;
        version_ = 0;
    }
    
    size_t SerializedSize() const
    {
        return sizeof(high_bandwidth_mode_) + sizeof(version_);
    }
    
    util::Hash256 GetHash() const
    {
        return crypto::GetHash(*this);
    }
    
    //-------------------------------------------------------------------------
    bool operator==(const SendCmpct& b) const
    {
        return (high_bandwidth_mode_ == b.high_bandwidth_mode_ &&
                version_ == b.version_);
    }
    
    bool operator!=(const SendCmpct &b) const
    {
        return !(*this == b);
    }
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& out) const 
    {
        util::Serializer<Stream> serializer(out);
        serializer.SerialWrite(high_bandwidth_mode_);
        serializer.SerialWrite(version_);
    }
    
    template <typename Stream>
    void Deserialize(Stream& in)
    {
        util::Deserializer<Stream> deserializer(in);
        deserializer.SerialRead(&high_bandwidth_mode_);
        deserializer.SerialRead(&version_);
    }
    
    //-------------------------------------------------------------------------
    bool high_bandwidth_mode() const
    {
        return high_bandwidth_mode_;
    }
    
    void set_high_bandwidth_mode(bool high_bandwidth_mode)
    {
        high_bandwidth_mode_ = high_bandwidth_mode;
    }
    
    uint64_t version() const
    {
        return version_;
    }
    
    void set_version(uint64_t version)
    {
        version_ = version;
    }
    
private:
    bool high_bandwidth_mode_ = false;
    uint64_t version_ = 0;
};

} // namespace protocol
} // namespace network
} // namespace btclite

#endif // BTCLITE_PROTOCOL_SEND_COMPACT_H
