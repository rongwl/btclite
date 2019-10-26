#ifndef BTCLITE_PROTOCOL_VERSION_H
#define BTCLITE_PROTOCOL_VERSION_H


#include "message.h"
#include "network_address.h"
#include "stream.h"


namespace btclite {
namespace network {
namespace protocol {

enum VersionCode : uint32_t {
    // initial proto version, to be increased after version/verack negotiation
    kInitProtoVersion = 209,
    
    // timestamp field added to NetAddr, starting with this version;
    // if possible, avoid requesting addresses nodes older than this
    kAddrTimeVersion = 31402,
    
    // In this version, 'getheaders' was introduced.
    kGetheadersVersion = 31800,
    
    // BIP 0031, pong message, is enabled for all versions AFTER this one
    kBip31Version = 60000,
    
    // BIP 0037, whether the remote peer should announce relayed transactions or not
    kRelayedTxsVersion = 70001,
    
    // "filter*" commands are disabled without kNodeBloom after and including this version
    kNoBloomVersion = 70011,
    
    // "sendheaders" command and announcing blocks with headers starts with this version
    kSendheadersVersion = 70012,
    
    // "feefilter" tells peers to filter invs to you by fee starts with this version
    kShortIdsBlocksVersion = 70014,
    
    // not banning for invalid compact blocks starts with this version
    kInvalidCbNoBanVersion = 70015
};

constexpr VersionCode kProtocolVersion = kInvalidCbNoBanVersion;
constexpr VersionCode kMinPeerProtoVersion = kGetheadersVersion;

class Version : public MessageData {
public:
    Version() = default;
    
    Version(uint32_t version, uint64_t services, uint64_t timestamp,
            const btclite::network::NetAddr& addr_recv,
            const btclite::network::NetAddr& addr_from,
            uint64_t nonce, const std::string& user_agent,
            uint32_t start_height, bool relay)
        : version_(version), services_(services), timestamp_(timestamp),
          addr_recv_(addr_recv), addr_from_(addr_from), nonce_(nonce),
          user_agent_(user_agent), start_height_(start_height), relay_(relay) {}
    
    Version(uint32_t version, uint64_t services, uint64_t timestamp,
            btclite::network::NetAddr&& addr_recv,
            btclite::network::NetAddr&& addr_from,
            uint64_t nonce, std::string&& user_agent,
            uint32_t start_height, bool relay) noexcept
        : version_(version), services_(services), timestamp_(timestamp),
          addr_recv_(std::move(addr_recv)), addr_from_(std::move(addr_from)),
          nonce_(nonce), user_agent_(std::move(user_agent)),
          start_height_(start_height), relay_(relay) {}
    
    //-------------------------------------------------------------------------
    bool RecvHandler(std::shared_ptr<Node> src_node) const;
    
    std::string Command() const
    {
        return kMsgVersion;
    }
    
    //-------------------------------------------------------------------------
    bool IsValid() const;
    void Clear();
    size_t SerializedSize() const;
    
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
    uint32_t version() const
    {
        return version_;
    }
    void set_version(uint32_t version)
    {
        version_ = version;
    }

    uint64_t services() const
    {
        return services_;
    }
    void set_services(uint64_t services)
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
    
    const btclite::network::NetAddr& addr_recv() const
    {
        return addr_recv_;
    }
    void set_addr_recv(const btclite::network::NetAddr& addr)
    {
        addr_recv_ = addr;
    }
    void set_addr_recv(btclite::network::NetAddr&& addr) noexcept
    {
        addr_recv_ = std::move(addr);
    }

    const btclite::network::NetAddr& addr_from() const
    {
        return addr_from_;
    }
    void set_addr_from(const btclite::network::NetAddr& addr)
    {
        addr_from_ = addr;
    }
    void set_addr_from(btclite::network::NetAddr&& addr) noexcept
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
    uint32_t version_ = 0;
    uint64_t services_ = 0;
    uint64_t timestamp_ = 0;
    btclite::network::NetAddr addr_recv_;
    
    // Fields below require version ≥ 106
    btclite::network::NetAddr addr_from_;
    uint64_t nonce_ = 0;
    std::string user_agent_;
    uint32_t start_height_ = 0;
    
    // Fields below require version ≥ 70001
    bool relay_ = false;
};

template <typename Stream>
void Version::Serialize(Stream& out) const
{
    Serializer<Stream> serializer(out);
    
    serializer.SerialWrite(version_);
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
    if (version_ >= kRelayedTxsVersion)
        serializer.SerialWrite(relay_);
}

template <typename Stream>
void Version::Deserialize(Stream& in)
{
    Deserializer<Stream> deserializer(in);
    
    deserializer.SerialRead(&version_);
    deserializer.SerialRead(&services_);
    deserializer.SerialRead(&timestamp_);
    deserializer.SerialRead(&addr_recv_);
    deserializer.SerialRead(&addr_from_);
    deserializer.SerialRead(&nonce_);
    deserializer.SerialRead(&user_agent_);
    if (user_agent_.size() > kMaxSubVersionSize)
        user_agent_.resize(kMaxSubVersionSize);
    deserializer.SerialRead(&start_height_);
    if (version_ >= kRelayedTxsVersion)
        deserializer.SerialRead(&relay_);
}

} // namespace protocol
} // namespace network
} // namespace btclite

#endif // BTCLITE_PROTOCOL_VERSION_H
