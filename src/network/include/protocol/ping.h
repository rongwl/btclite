#ifndef BTCLITE_PROTOCOL_PING_H
#define BTCLITE_PROTOCOL_PING_H


#include "message.h"
#include "protocol/version.h"


namespace btclite {
namespace network {
namespace protocol {

class ping : public MessageData {
public:
    ping() = default;
    
    explicit ping(uint64_t nonce)
        : nonce_(nonce) {}
    
    ping(uint64_t nonce, uint32_t protocol_version)
        : nonce_(nonce), protocol_version_(protocol_version) {}
    
    //-------------------------------------------------------------------------
    bool RecvHandler(std::shared_ptr<Node> src_node) const;
    static void PingTimeoutCb(std::shared_ptr<Node> node);
    
    std::string Command() const
    {
        return kMsgPing;
    }
    
    bool IsValid() const
    {
        if (protocol_version_ < VersionCode::kBip31Version)
            return true;
        return (nonce_ != 0);
    }
    
    void Clear()
    {
        nonce_ = 0;
    }
    
    size_t SerializedSize() const
    {
        if (protocol_version_ < VersionCode::kBip31Version)
            return 0;
        return sizeof(nonce_);
    }
    
    //-------------------------------------------------------------------------
    bool operator==(const ping& b) const
    {
        return nonce_ == b.nonce_;
    }
    
    bool operator!=(const ping& b) const
    {
        return !(*this == b);
    }
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& out) const;    
    template <typename Stream>
    void Deserialize(Stream& in);
    
    //-------------------------------------------------------------------------
    uint64_t nonce() const
    {
        return nonce_;
    }
    
    void set_nonce(uint64_t nonce)
    {
        nonce_ = nonce;
    }
    
    uint32_t protocol_version() const
    {
        return protocol_version_;
    }
    
    void set_protocol_version(uint32_t version)
    {
        protocol_version_ = version;
    }
    
private:
    uint64_t nonce_ = 0;
    
    // different version for different ping
    uint32_t protocol_version_ = kProtocolVersion;
};

template <typename Stream>
void ping::Serialize(Stream& out) const
{
    if (protocol_version_ >= VersionCode::kBip31Version) {
        Serializer<Stream> serializer(out);
        serializer.SerialWrite(nonce_);
    }
}

template <typename Stream>
void ping::Deserialize(Stream& in)
{
    if (protocol_version_ >= VersionCode::kBip31Version) {
        Deserializer<Stream> deserializer(in);
        deserializer.SerialRead(&nonce_);
    }
}

using Ping = Hashable<ping>;

} // namespace protocol
} // namespace network
} // namespace btclite


#endif // BTCLITE_PROTOCOL_PING_H
