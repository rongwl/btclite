#ifndef BTCLITE_NETWORK_ADDRESS_H
#define BTCLITE_NETWORK_ADDRESS_H

#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 

#include "network_address.pb.h"


namespace btclite {

enum AddrFamily {
    AF_UNROUTABLE = 0,
    AF_IPV4,
    AF_IPV6,
    AF_TOR,
    AF_INTERNAL,

    AF_MAXIMUM,
};
    
class NetAddr {
public:
    static constexpr size_t ip_byte_size = 16;
    static constexpr size_t ip_uint32_size = 4;

    NetAddr()
        : proto_addr_()
    {
        proto_addr_.mutable_ip()->Resize(ip_uint32_size, 0);
    }
    
    explicit NetAddr(const struct sockaddr_in& addr);    
    explicit NetAddr(const struct sockaddr_in6& addr6);    
    explicit NetAddr(const struct sockaddr_storage& addr);
    explicit NetAddr(const proto_netaddr::NetAddr& proto_addr)
        : proto_addr_(proto_addr) {}
    
    //-------------------------------------------------------------------------
    bool IsIpv4() const;
    bool IsIpv6() const;
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
    
    //-------------------------------------------------------------------------
    uint8_t GetByte(int n) const;
    void SetByte(int n, uint8_t value);
    void SetNByte(const uint8_t *src, size_t n);
    uint32_t GetIpv4() const; // return ip in network byte order
    void SetIpv4(uint32_t net_byte_order_ip);
    int GetIpv6(uint8_t *out) const;
    void SetIpv6(const uint8_t *src);
    void GetGroup(std::vector<uint8_t> *out) const;
    
    //-------------------------------------------------------------------------
    bool operator==(const NetAddr& b) const
    {
        return (std::memcmp(this->proto_addr_.ip().data(), b.proto_addr().ip().data(), ip_byte_size) == 0 &&
                this->proto_addr_.port() == b.proto_addr().port());
    }
    
    bool operator!=(const NetAddr& b) const
    {
        return !(*this == b);
    }
    
    //-------------------------------------------------------------------------
    const proto_netaddr::NetAddr& proto_addr() const
    {
        return proto_addr_;
    }
    
    proto_netaddr::NetAddr *mutable_proto_addr()
    {
        return &proto_addr_;
    }

private:
    proto_netaddr::NetAddr proto_addr_;
    
#define ASSERT_SIZE() assert(proto_addr_.ip().size() == 4)
#define ASSERT_RANGE(N) assert(N >= 0 && N <= 15)
#define ASSERT_VALID_BYTE(N) ASSERT_SIZE(); \
                             ASSERT_RANGE(N)
};

} // namespace btclite

class SubNet {
public:
    static constexpr size_t netmask_byte_size = 16;    
    static int NetmaskBits(uint8_t x);
    
    SubNet()
        : net_addr_(), valid_(false)
    {
        std::memset(netmask_, 0, sizeof(netmask_));
    }
    
    explicit SubNet(const btclite::NetAddr& addr)
        : net_addr_(addr), valid_(addr.IsValid())
    {
        std::memset(netmask_, 0xff, sizeof(netmask_));
    }
    
    explicit SubNet(btclite::NetAddr&& addr) noexcept
        : net_addr_(std::move(addr)), valid_(addr.IsValid())
    {
        std::memset(netmask_, 0xff, sizeof(netmask_));
    }
    
    SubNet(const btclite::NetAddr& addr, int32_t mask);
    SubNet(const btclite::NetAddr &addr, const btclite::NetAddr &mask);
    
    //-------------------------------------------------------------------------
    bool IsValid() const
    {
        return valid_;
    }
    
    void Clear()
    {
        net_addr_.Clear();
        std::memset(netmask_, 0, sizeof(netmask_));
        valid_ = false;
    }
    
    bool Match(const btclite::NetAddr& addr) const;
    std::string ToString() const;
    
    //-------------------------------------------------------------------------
    friend bool operator==(const SubNet& a, const SubNet& b)
    {
        return (std::memcmp(a.net_addr().proto_addr().ip().data(),
                            b.net_addr().proto_addr().ip().data(),
                            btclite::NetAddr::ip_byte_size) == 0);
    }
    
    friend bool operator!=(const SubNet& a, const SubNet& b)
    {
        return !(a == b);
    }
    
    //-------------------------------------------------------------------------
    const btclite::NetAddr& net_addr() const
    {
        return net_addr_;
    }
    
    const uint8_t* const netmask() const
    {
        return netmask_;
    }
    
private:
    btclite::NetAddr net_addr_;
    uint8_t netmask_[netmask_byte_size];
    bool valid_;
};


#endif // BTCLITE_NETWORK_ADDRESS_H
