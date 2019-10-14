#ifndef BTCLITE_PROTOCOL_PING_H
#define BTCLITE_PROTOCOL_PING_H


#include "message.h"


namespace btclite {
namespace network {
namespace protocol {

class Ping : public MessageData {
public:
    static const std::string kCommand;
    
    //-------------------------------------------------------------------------
    Ping()
        : nonce_(0) {}
    
    explicit Ping(uint64_t nonce)
        : nonce_(nonce) {}
    
    Ping(const Ping& ping)
        : nonce_(ping.nonce_) {}
    
    explicit Ping(const uint8_t *raw);
    
    //-------------------------------------------------------------------------
    bool RecvHandler(std::shared_ptr<Node> src_node) const;
    
    bool IsValid() const
    {
        return (nonce_ != 0);
    }
    
    void Clear()
    {
        nonce_ = 0;
    }
    
    size_t SerializedSize() const
    {
        return sizeof(nonce_);
    }
    
    //-------------------------------------------------------------------------
    bool operator==(const Ping& b) const
    {
        return nonce_ == b.nonce_;
    }
    
    bool operator!=(const Ping& b) const
    {
        return !(*this == b);
    }
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& out) const
    {
        Serializer<Stream> serializer(out);
        serializer.SerialWrite(nonce_);
    }
    
    template <typename Stream>
    void Deserialize(Stream& in)
    {
        Deserializer<Stream> deserializer(in);
        deserializer.SerialRead(&nonce_);
    }
    
    //-------------------------------------------------------------------------
    uint64_t nonce() const
    {
        return nonce_;
    }
    
    void set_nonce(uint64_t nonce)
    {
        nonce_ = nonce;
    }
    
private:
    uint64_t nonce_;
};

} // namespace protocol
} // namespace network
} // namespace btclite


#endif // BTCLITE_PROTOCOL_PING_H
