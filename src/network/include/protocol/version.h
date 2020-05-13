#ifndef BTCLITE_PROTOCOL_VERSION_H
#define BTCLITE_PROTOCOL_VERSION_H


#include "message.h"
#include "network_address.h"
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
            uint32_t start_height, bool relay)
        : protocol_version_(version), services_(services), timestamp_(timestamp),
          addr_recv_(addr_recv), addr_from_(addr_from), nonce_(nonce),
          user_agent_(user_agent), start_height_(start_height), relay_(relay) {}
    
    Version(ProtocolVersion version, ServiceFlags services, uint64_t timestamp,
            NetAddr&& addr_recv,
            NetAddr&& addr_from,
            uint64_t nonce, std::string&& user_agent,
            uint32_t start_height, bool relay) noexcept
        : protocol_version_(version), services_(services), timestamp_(timestamp),
          addr_recv_(std::move(addr_recv)), addr_from_(std::move(addr_from)),
          nonce_(nonce), user_agent_(std::move(user_agent)),
          start_height_(start_height), relay_(relay) {}
    
    //-------------------------------------------------------------------------
    bool RecvHandler(std::shared_ptr<Node> src_node, 
                     uint32_t magic, bool advertise_local) const;
    
    std::string Command() const
    {
        return msg_command::kMsgVersion;
    }
    
    bool IsValid() const;
    void Clear();
    size_t SerializedSize() const;
    
    util::Hash256 GetHash() const
    {
        return crypto::GetHash(*this);
    }
    
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
    ProtocolVersion protocol_version() const
    {
        return protocol_version_;
    }
    void set_protocol_version(ProtocolVersion version)
    {
        protocol_version_ = version;
    }

    ServiceFlags services() const
    {
        return services_;
    }
    void set_services(ServiceFlags services)
    {
        services_ = services;
    }

    uint64_t timestamp() const
    {
        return timestamp_;
    }
    void set_timestamp(uint64_t timestamp)
    {
        timestamp_ = timestamp;
    }
    
    const NetAddr& addr_recv() const
    {
        return addr_recv_;
    }
    void set_addr_recv(const NetAddr& addr)
    {
        addr_recv_ = addr;
    }
    void set_addr_recv(NetAddr&& addr) noexcept
    {
        addr_recv_ = std::move(addr);
    }

    const NetAddr& addr_from() const
    {
        return addr_from_;
    }
    void set_addr_from(const NetAddr& addr)
    {
        addr_from_ = addr;
    }
    void set_addr_from(NetAddr&& addr) noexcept
    {
        addr_from_ = std::move(addr);
    }
    
    uint64_t nonce() const
    {
        return nonce_;
    }
    void set_nonce(uint64_t nonce)
    {
        nonce_ = nonce;
    }

    const std::string& user_agent() const
    {
        return user_agent_;
    }
    void set_user_agent(const std::string& agent)
    {
        user_agent_ = agent;
    }
    void set_user_agent(std::string&& agent) noexcept
    {
        user_agent_ = std::move(agent);
    }

    uint32_t start_height() const
    {
        return start_height_;
    }
    void set_start_height(uint32_t height)
    {
        start_height_ = height;
    }

    bool relay() const
    {
        return relay_;
    }
    void set_relay(bool relay)
    {
        relay_ = relay;
    }

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
