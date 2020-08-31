#ifndef BTCLITE_PROTOCOL_PONG_H
#define BTCLITE_PROTOCOL_PONG_H


#include "message.h"
#include "protocol/version.h"


namespace btclite {
namespace network {
namespace protocol {

class Pong {
public:
    Pong() = default;
    
    explicit Pong(uint64_t nonce);
    
    //-------------------------------------------------------------------------
    bool RecvHandler(std::shared_ptr<Node> src_node) const;    
    std::string Command() const;
    bool IsValid() const;
    void Clear();
    size_t SerializedSize() const;
    util::Hash256 GetHash() const;
    
    //-------------------------------------------------------------------------
    bool operator==(const Pong& b) const;
    bool operator!=(const Pong& b) const;
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& out) const
    {
        util::Serializer<Stream> serializer(out);
        serializer.SerialWrite(nonce_);
    }
    
    template <typename Stream>
    void Deserialize(Stream& in)
    {
        util::Deserializer<Stream> deserializer(in);
        deserializer.SerialRead(&nonce_);
    }
    
    //-------------------------------------------------------------------------
    uint64_t nonce() const;
    void set_nonce(uint64_t nonce);
    
private:
    uint64_t nonce_ = 0;
};

} // namespace protocol
} // namespace network
} // namespace btclite

#endif // BTCLITE_PROTOCOL_PONG_H
