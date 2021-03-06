#ifndef BTCLITE_NETWORK_ADDRESS_H
#define BTCLITE_NETWORK_ADDRESS_H

#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 

#include "constants.h"
#include "hash.h"
#include "network_address.pb.h"
#include "serialize.h"


namespace btclite {
namespace network {

using IpAddr = util::Bytes<kIpByteSize>;

enum AddrFamily {
    kAfUnroutable = 0,
    kAfIpv4,
    kAfIpv6,
    kAfTor,
    kAfInternal,
    kAfTeredo,
    
    kAfMaximum,
};

enum AddrCategory {
    kAcNone,    // unknown
    kAcLocalIf, // address a local interface listens on
    kAcBind,    // address explicit bound to
    kAcUpnp,    // address reported by upnp
    kAcManual,  // address explicitly specified (-externalip=)

    kAcMax
};

    
class NetAddr {
public:
    NetAddr();
    
    NetAddr(const struct sockaddr_in& addr);    
    NetAddr(const struct sockaddr_in6& addr6);    
    NetAddr(const struct sockaddr_storage& addr);
    
    NetAddr(const proto_netaddr::NetAddr& proto_addr);
    NetAddr(proto_netaddr::NetAddr&& proto_addr) noexcept;
    
    NetAddr(uint32_t timestamp, uint64_t services, IpAddr& ip, uint16_t port);
    NetAddr(uint32_t timestamp, uint64_t services, IpAddr&& ip, uint16_t port) noexcept;
    
    //-------------------------------------------------------------------------
    bool IsIpv4() const;
    bool IsIpv6() const;
    bool IsTor() const;
    bool IsRFC1918() const; // IPv4 private networks (10.0.0.0/8, 192.168.0.0/16, 172.16.0.0/12)
    bool IsRFC2544() const; // IPv4 inter-network communications (192.18.0.0/15)
    bool IsRFC6598() const; // IPv4 ISP-level NAT (100.64.0.0/10)
    bool IsRFC5737() const; // IPv4 documentation addresses (192.0.2.0/24, 198.51.100.0/24, 203.0.113.0/24)
    bool IsRFC3849() const; // IPv6 documentation address (2001:0DB8::/32)
    bool IsRFC3927() const; // IPv4 autoconfig (169.254.0.0/16)
    bool IsRFC3964() const; // IPv6 6to4 tunnelling (2002::/16)
    bool IsRFC4193() const; // IPv6 unique local (FC00::/7)
    bool IsRFC4380() const; // IPv6 Teredo tunnelling (2001::/32)
    bool IsRFC4843() const; // IPv6 ORCHID (2001:10::/28)
    bool IsRFC4862() const; // IPv6 autoconfig (FE80::/64)
    bool IsRFC6052() const; // IPv6 well-known prefix (64:FF9B::/96)
    bool IsRFC6145() const; // IPv6 IPv4-translated address (::FFFF:0:0:0/96)
    bool IsLocal() const;
    bool IsRoutable() const;
    bool IsInternal() const;
    bool IsValid() const;
    
    //-------------------------------------------------------------------------
    void Clear();
    std::string ToString() const;
    bool ToSockAddr(struct sockaddr* out) const;
    bool FromSockAddr(const struct sockaddr *in);    
    size_t SerializedSize() const;
    
    //-------------------------------------------------------------------------
    uint8_t GetByte(int n) const;
    void SetByte(int n, uint8_t value);
    void SetNByte(const uint8_t *src, size_t n);
    uint32_t GetIpv4() const; // return ip in network byte order
    void SetIpv4(uint32_t net_byte_order_ip);
    bool GetIpv6(uint8_t *out) const;
    void SetIpv6(const uint8_t *src);
    void GetGroup(std::vector<uint8_t> *out) const;
    bool SetInternal(const std::string& name);
    AddrFamily GetFamily() const;
    int GetReachability(const NetAddr& addr_partner) const;
    util::Hash256 GetHash() const;
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& out) const;
    template <typename Stream>
    void Deserialize(Stream& in);
    
    //-------------------------------------------------------------------------
    friend bool operator==(const NetAddr& a, const NetAddr& b)
    {
        return (std::memcmp(a.proto_addr_.ip().begin(), b.proto_addr_.ip().begin(), kIpByteSize) == 0 &&
                a.proto_addr_.port() == b.proto_addr_.port());
    }
    
    friend bool operator!=(const NetAddr& a, const NetAddr& b)
    {
        return !(a == b);
    }
    
    friend bool operator<(const NetAddr& a, const NetAddr& b)
    {
        return (std::memcmp(a.proto_addr_.ip().begin(), b.proto_addr_.ip().begin(), kIpByteSize)
                < 0);
    }
    
    //-------------------------------------------------------------------------    
    uint16_t port() const;    
    void set_port(uint16_t port);
    
    uint32_t scope_id() const;    
    void set_scope_id(uint32_t id);
    
    uint64_t services() const;    
    void set_services(uint64_t services);
    
    uint32_t timestamp() const;    
    void set_timestamp(uint32_t timestamp);
    
    const proto_netaddr::NetAddr& proto_addr() const;

private:
    size_t ip_uint32_size = 4;
    
    proto_netaddr::NetAddr proto_addr_;
    
    AddrFamily GetExtFamily() const;
    
#define ASSERT_SIZE() assert(proto_addr_.ip().size() == 4)
#define ASSERT_RANGE(N) assert(N >= 0 && N <= 15)
#define ASSERT_VALID_BYTE(N) ASSERT_SIZE(); \
                             ASSERT_RANGE(N)
};

template <typename Stream>
void NetAddr::Serialize(Stream& out) const
{
    util::Serializer<Stream> serializer(out);
    uint16_t port = static_cast<uint16_t>(proto_addr_.port());
    
    serializer.SerialWrite(proto_addr_.timestamp());
    serializer.SerialWrite(proto_addr_.services());
    serializer.SerialWrite(proto_addr_.ip());
    serializer.SerialWrite(port);
}

template <typename Stream>
void NetAddr::Deserialize(Stream& in)
{
    util::Deserializer<Stream> deserializer(in);
    uint32_t timestamp;
    uint64_t services;
    uint16_t port;
    
    deserializer.SerialRead(&timestamp);
    deserializer.SerialRead(&services);
    deserializer.SerialRead(proto_addr_.mutable_ip());
    deserializer.SerialRead(&port);
    
    proto_addr_.set_timestamp(timestamp);
    proto_addr_.set_services(services);
    proto_addr_.set_port(port);
}

class SubNet {
public:
    static constexpr size_t netmask_byte_size = 16;
    
    SubNet();
    
    SubNet(const NetAddr& addr);
    SubNet(NetAddr&& addr) noexcept;
    
    SubNet(const NetAddr& addr, int32_t mask);
    SubNet(const NetAddr &addr, const NetAddr &mask);
    
    //-------------------------------------------------------------------------
    bool IsValid() const;    
    void Clear();    
    bool Match(const NetAddr& addr) const;
    std::string ToString() const;
    
    //-------------------------------------------------------------------------
    friend bool operator==(const SubNet& a, const SubNet& b)
    {
        return (a.net_addr() == b.net_addr());
    }
    
    friend bool operator!=(const SubNet& a, const SubNet& b)
    {
        return !(a == b);
    }
    
    //-------------------------------------------------------------------------
    const NetAddr& net_addr() const;
    
    const uint8_t* const netmask() const;
    
private:
    NetAddr net_addr_;
    uint8_t netmask_[netmask_byte_size];
    bool valid_;
    
    int NetmaskBits(uint8_t x) const;
};

} // namespace network
} // namespace btclite


#endif // BTCLITE_NETWORK_ADDRESS_H
