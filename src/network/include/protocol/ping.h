#ifndef BTCLITE_PROTOCOL_PING_H
#define BTCLITE_PROTOCOL_PING_H


#include "message.h"
#include "protocol/version.h"


namespace btclite {
namespace network {
namespace protocol {

class Ping {
public:
    Ping() = default;
    
    explicit Ping(uint64_t nonce);
    
    Ping(uint64_t nonce, uint32_t protocol_version);
    
    //-------------------------------------------------------------------------
    bool RecvHandler(std::shared_ptr<Node> src_node, uint32_t magic) const;    
    std::string Command() const;
    bool IsValid() const;
    void Clear();
    size_t SerializedSize() const;
    util::Hash256 GetHash() const;
    
    //-------------------------------------------------------------------------
    bool operator==(const Ping& b) const;
    bool operator!=(const Ping& b) const;
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& out) const;    
    template <typename Stream>
    void Deserialize(Stream& in);
    
    //-------------------------------------------------------------------------
    uint64_t nonce() const;
    void set_nonce(uint64_t nonce);
    
    uint32_t protocol_version() const;
    void set_protocol_version(uint32_t version);
    
private:
    uint64_t nonce_ = 0;
    
    // different version for different ping
    uint32_t protocol_version_ = kProtocolVersion;
};

template <typename Stream>
void Ping::Serialize(Stream& out) const
{
    if (protocol_version_ >= kBip31Version) {
        util::Serializer<Stream> serializer(out);
        serializer.SerialWrite(nonce_);
    }
}

template <typename Stream>
void Ping::Deserialize(Stream& in)
{
    if (protocol_version_ >= kBip31Version) {
        util::Deserializer<Stream> deserializer(in);
        deserializer.SerialRead(&nonce_);
    }
}

} // namespace protocol
} // namespace network
} // namespace btclite


#endif // BTCLITE_PROTOCOL_PING_H
