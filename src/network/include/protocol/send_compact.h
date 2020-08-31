#ifndef BTCLITE_PROTOCOL_SEND_COMPACT_H
#define BTCLITE_PROTOCOL_SEND_COMPACT_H


#include "message.h"


namespace btclite {
namespace network {
namespace protocol {

class SendCmpct {
public:
    SendCmpct() = default;
    
    SendCmpct(bool high_bandwidth_mode, uint64_t version);
    
    //-------------------------------------------------------------------------
    bool RecvHandler(std::shared_ptr<Node> src_node) const;    
    std::string Command() const;
    bool IsValid() const;
    void Clear();
    size_t SerializedSize() const;
    util::Hash256 GetHash() const;
    
    //-------------------------------------------------------------------------
    bool operator==(const SendCmpct& b) const;
    bool operator!=(const SendCmpct &b) const;
    
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
    bool high_bandwidth_mode() const;
    void set_high_bandwidth_mode(bool high_bandwidth_mode);
    
    uint64_t version() const;
    void set_version(uint64_t version);
    
private:
    bool high_bandwidth_mode_ = false;
    uint64_t version_ = 0;
};

} // namespace protocol
} // namespace network
} // namespace btclite

#endif // BTCLITE_PROTOCOL_SEND_COMPACT_H
