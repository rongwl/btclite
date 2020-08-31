#ifndef BTCLITE_PROTOCOL_VERSION_H
#define BTCLITE_PROTOCOL_VERSION_H


#include "message.h"
#include "net.h"
#include "stream.h"

#include "btclite-config.h"


namespace btclite {
namespace network {
namespace protocol {

inline std::string FormatUserAgent()
{
    std::stringstream ss;
    ss << "/" << PACKAGE_NAME << ":" << PACKAGE_VERSION << "/";
    return ss.str();
}

constexpr ProtocolVersion kProtocolVersion = kInvalidCbNoBanVersion;
constexpr ProtocolVersion kMinPeerProtoVersion = kGetheadersVersion;


class Version {
public:    
    Version() = default;
    
    Version(ProtocolVersion version, ServiceFlags services, uint64_t timestamp,
            const NetAddr& addr_recv, const NetAddr& addr_from,
            uint64_t nonce, const std::string& user_agent,
            uint32_t start_height, bool relay);    
    Version(ProtocolVersion version, ServiceFlags services, uint64_t timestamp,
            NetAddr&& addr_recv, NetAddr&& addr_from,
            uint64_t nonce, std::string&& user_agent,
            uint32_t start_height, bool relay) noexcept;
    
    //-------------------------------------------------------------------------
    bool RecvHandler(std::shared_ptr<Node> src_node, uint32_t magic, 
                     bool advertise_local, const LocalService& local_service,
                     uint32_t height, Peers *ppeers) const;    
    std::string Command() const;    
    bool IsValid() const;
    void Clear();
    size_t SerializedSize() const;    
    util::Hash256 GetHash() const;
    
    //-------------------------------------------------------------------------
    Version& operator=(const Version& b);
    Version& operator=(Version&& b) noexcept;
    bool operator==(const Version& b) const;
    bool operator!=(const Version& b) const;
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& out) const;
    template <typename Stream>
    void Deserialize(Stream& in);
    
    //-------------------------------------------------------------------------
    ProtocolVersion protocol_version() const;
    void set_protocol_version(ProtocolVersion version);

    ServiceFlags services() const;
    void set_services(ServiceFlags services);

    uint64_t timestamp() const;
    void set_timestamp(uint64_t timestamp);
    
    const NetAddr& addr_recv() const;
    void set_addr_recv(const NetAddr& addr);
    void set_addr_recv(NetAddr&& addr) noexcept;

    const NetAddr& addr_from() const;
    void set_addr_from(const NetAddr& addr);
    void set_addr_from(NetAddr&& addr) noexcept;
    
    uint64_t nonce() const;
    void set_nonce(uint64_t nonce);

    const std::string& user_agent() const;
    void set_user_agent(const std::string& agent);
    void set_user_agent(std::string&& agent) noexcept;

    uint32_t start_height() const;
    void set_start_height(uint32_t height);

    bool relay() const;
    void set_relay(bool relay);

private:
    ProtocolVersion protocol_version_ = kUnknownProtoVersion;
    ServiceFlags services_ = kNodeNone;
    uint64_t timestamp_ = 0;
    NetAddr addr_recv_;
    
    // Fields below require version ≥ 106
    NetAddr addr_from_;
    uint64_t nonce_ = 0;
    std::string user_agent_;
    uint32_t start_height_ = 0;
    
    // Fields below require version ≥ 70001
    bool relay_ = false;
};

template <typename Stream>
void Version::Serialize(Stream& out) const
{
    util::Serializer<Stream> serializer(out);
    
    serializer.SerialWrite(protocol_version_);
    serializer.SerialWrite(services_);
    serializer.SerialWrite(timestamp_);
    serializer.SerialWrite(addr_recv_);
    serializer.SerialWrite(addr_from_);
    serializer.SerialWrite(nonce_);
    if (user_agent_.size() > kMaxSubVersionSize)
        serializer.SerialWrite(user_agent_.substr(0, kMaxSubVersionSize));
    else
        serializer.SerialWrite(user_agent_);
    serializer.SerialWrite(start_height_);
    if (protocol_version_ >= kRelayedTxsVersion)
        serializer.SerialWrite(relay_);
}

template <typename Stream>
void Version::Deserialize(Stream& in)
{
    util::Deserializer<Stream> deserializer(in);
    
    deserializer.SerialRead(&protocol_version_);
    deserializer.SerialRead(&services_);
    deserializer.SerialRead(&timestamp_);
    deserializer.SerialRead(&addr_recv_);
    deserializer.SerialRead(&addr_from_);
    deserializer.SerialRead(&nonce_);
    deserializer.SerialRead(&user_agent_);
    if (user_agent_.size() > kMaxSubVersionSize)
        user_agent_.resize(kMaxSubVersionSize);
    deserializer.SerialRead(&start_height_);
    if (protocol_version_ >= kRelayedTxsVersion)
        deserializer.SerialRead(&relay_);
}


} // namespace protocol
} // namespace network
} // namespace btclite

#endif // BTCLITE_PROTOCOL_VERSION_H
