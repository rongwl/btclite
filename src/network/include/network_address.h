#ifndef BTCLITE_NETWORK_ADDRESS_H
#define BTCLITE_NETWORK_ADDRESS_H

#include <cstring>

#include "network_address.pb.h"

namespace btclite {
    
class NetAddr {
public:
    static constexpr size_t ip_byte_size = 16;
    static constexpr size_t ip_uint32_size = 4;

    NetAddr()
        : addr_()
    {
        for (int i = 0; i < ip_uint32_size; i++)
            addr_.add_ip(0);
    }
    
    NetAddr(const proto_netaddr::NetAddr& addr)
        : addr_(addr) {}
    NetAddr(proto_netaddr::NetAddr&& addr) noexcept
        : addr_(std::move(addr)) {}
    
    NetAddr(const NetAddr& addr)
        : addr_(addr.addr_) {}
    NetAddr(NetAddr&& addr) noexcept
        : addr_(std::move(addr.addr_)) {}
    
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
    uint8_t GetByte(int n) const;
    void SetByte(int n, uint8_t value);
    void SetNByte(const uint8_t *src, size_t n);
    uint32_t GetIpv4() const;
    void SetIpv4(uint32_t ip);
    int GetIpv6(uint8_t *out) const;
    void SetIpv6(const uint8_t *src);
    
    //-------------------------------------------------------------------------
    NetAddr& operator=(const NetAddr& b)
    {
        addr_ = b.addr_;
        return *this;
    }
    
    NetAddr& operator=(NetAddr&& b) noexcept
    {
        addr_ = std::move(b.addr_);
        return *this;
    }
    
    bool operator==(const NetAddr& b) const
    {
        return (this->addr_.timestamp() == b.addr().timestamp() &&
                this->addr_.services() == b.addr().services() &&
                std::memcmp(this->addr_.ip().data(), b.addr().ip().data(), ip_byte_size) == 0 &&
                this->addr_.port() == b.addr().port());
    }
    
    bool operator!=(const NetAddr& b) const
    {
        return !(*this == b);
    }
    
    //-------------------------------------------------------------------------
    const proto_netaddr::NetAddr& addr() const
    {
        return addr_;
    }
    
    void set_addr(const proto_netaddr::NetAddr& addr)
    {
        addr_ = addr;
    }
    
    void set_addr(proto_netaddr::NetAddr&& addr) noexcept
    {
        addr_ = std::move(addr);
    }
    
private:
    proto_netaddr::NetAddr addr_;
    
#define ASSERT_SIZE() assert(addr_.ip().size() == 4)
#define ASSERT_RANGE(N) assert(N >= 0 && N <= 15)
#define ASSERT_VALID_BYTE(N) ASSERT_SIZE(); \
                             ASSERT_RANGE(N)
};

}

#endif // BTCLITE_NETWORK_ADDRESS_H
