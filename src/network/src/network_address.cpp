#include <arpa/inet.h>
#include <botan/base32.h>
#include <sstream>

#include "network_address.h"
#include "constants.h"

namespace btclite {

bool NetAddr::IsIpv4() const
{
    ASSERT_SIZE();
    return (std::memcmp(addr_.ip().data(), pch_ipv4, sizeof(pch_ipv4)) == 0);
}

bool NetAddr::IsIpv6() const
{
    ASSERT_SIZE();
    return (!IsIpv4() && !IsInternal());
}

bool NetAddr::IsRFC1918() const
{
    return IsIpv4() && (
               GetByte(12) == 10 ||
               (GetByte(12) == 192 && GetByte(13) == 168) ||
               (GetByte(12) == 172 && (GetByte(13) >= 16 && GetByte(13) <= 31)));
    return true;
}

bool NetAddr::IsRFC2544() const
{
    return IsIpv4() && GetByte(12) == 198 && (GetByte(13) == 18 || GetByte(13) == 19);
}

bool NetAddr::IsRFC6598() const
{
    return IsIpv4() && GetByte(12) == 100 && GetByte(13) >= 64 && GetByte(13) <= 127;
}

bool NetAddr::IsRFC5737() const
{
    return IsIpv4() && ((GetByte(12) == 192 && GetByte(13) == 0 && GetByte(14) == 2) ||
                        (GetByte(12) == 198 && GetByte(13) == 51 && GetByte(14) == 100) ||
                        (GetByte(12) == 203 && GetByte(13) == 0 && GetByte(14) == 113));
}

bool NetAddr::IsRFC3849() const
{
    return IsIpv6() && GetByte(0) == 0x20 && GetByte(1) == 0x01 && GetByte(2) == 0x0D && GetByte(3) == 0xB8;
}

bool NetAddr::IsRFC3927() const
{
    return IsIpv4() && (GetByte(12) == 169 && GetByte(13) == 254);
}

bool NetAddr::IsRFC3964() const
{
    return IsIpv6() && (GetByte(0) == 0x20 && GetByte(1) == 0x02);
}

bool NetAddr::IsRFC4193() const
{
    return IsIpv6() && ((GetByte(0) & 0xFE) == 0xFC);
}

bool NetAddr::IsRFC4380() const
{
    return IsIpv6() && (GetByte(0) == 0x20 && GetByte(1) == 0x01 && GetByte(2) == 0 && GetByte(3) == 0);
}

bool NetAddr::IsRFC4843() const
{
    return IsIpv6() && 
           (GetByte(0) == 0x20 && GetByte(1) == 0x01 && GetByte(2) == 0x00 && (GetByte(3) & 0xF0) == 0x10);
}

bool NetAddr::IsRFC4862() const
{
    return IsIpv6() && (addr_.ip(0) == 0x80FE && addr_.ip(1) == 0);
}

bool NetAddr::IsRFC6052() const
{
    return IsIpv6() && (addr_.ip(0) == 0x9BFF6400 && addr_.ip(1) == 0);
}

bool NetAddr::IsRFC6145() const
{
    return IsIpv6() && (addr_.ip(0) == 0 && addr_.ip(1) == 0 && addr_.ip(2)  == 0xFFFF);
}

bool NetAddr::IsLocal() const
{
    // IPv4 loopback
    if (IsIpv4() && (GetByte(12) == 127 || GetByte(12) == 0))
        return true;

    // IPv6 loopback (::1/128)
    if (IsIpv6() && (addr_.ip(0) == 0 && addr_.ip(1) == 0 && addr_.ip(2) == 0 &&
                     addr_.ip(3) == 0x1000000))
        return true;

    return false;
}

bool NetAddr::IsRoutable() const
{
    return IsValid() && 
           !(IsRFC1918() || 
             IsRFC2544() ||
             IsRFC3927() ||
             IsRFC4862() ||
             IsRFC6598() ||
             IsRFC5737() ||
             IsRFC4193() ||
             IsRFC4843() ||
             IsLocal() ||
             IsInternal());
}

bool NetAddr::IsInternal() const
{
    ASSERT_SIZE();
    return (std::memcmp(addr_.ip().data(), btc_ip_prefix, sizeof(btc_ip_prefix)) == 0);
}

bool NetAddr::IsValid() const
{
    // Cleanup 3-byte shifted addresses caused by garbage in size field
    // of addr messages from versions before 0.2.9 checksum.
    // Two consecutive addr messages look like this:
    // header20 vectorlen3 addr26 addr26 addr26 header20 vectorlen3 addr26 addr26 addr26...
    // so if the first length field is garbled, it reads the second batch
    // of addr misaligned by 3 bytes.
    if (std::memcmp(addr_.ip().data(), pch_ipv4+3, sizeof(pch_ipv4)-3) == 0)
        return false;

    // unspecified IPv6 address (::/128)
    unsigned char ip6_none[ip_byte_size] = {};
    if (std::memcmp(addr_.ip().data(), ip6_none, ip_byte_size) == 0)
        return false;

    // documentation IPv6 address
    if (IsRFC3849())
        return false;

    if (IsInternal())
        return false;

    if (IsIpv4())
    {
        // INADDR_NONE
        if (addr_.ip(3) == INADDR_NONE)
            return false;

        // 0
        if (addr_.ip(3) == 0)
            return false;
    }

    return true;
}

std::string NetAddr::ToString() const
{
    std::stringstream ss;
    
    if (IsInternal())
        return Botan::base32_encode(reinterpret_cast<const uint8_t*>(addr_.ip().data()) + sizeof(btc_ip_prefix),
                                    ip_byte_size - sizeof(btc_ip_prefix)) + ".internal";

    if (IsIpv4())
        ss <<  GetByte(12) << "." << GetByte(13) << "." << GetByte(14) << "." << GetByte(15) << ":" << addr_.port();
    else
        ss << (GetByte(15) << 8 | GetByte(14)) << ":" << (GetByte(13) << 8 | GetByte(12)) << ":"
           << (GetByte(11) << 8 | GetByte(10)) << ":" << (GetByte(9) << 8 | GetByte(8)) << ":"
           << (GetByte(7) << 8 | GetByte(6)) << ":" << (GetByte(5) << 8 | GetByte(4)) << ":"
           << (GetByte(3) << 8 | GetByte(2)) << ":" << (GetByte(1) << 8 | GetByte(0)) << "," << addr_.port();
    
    return ss.str();
}

bool NetAddr::ToSockAddr(struct sockaddr* out, socklen_t *len) const
{
    if (IsIpv4()) {
        if (*len < static_cast<socklen_t>(sizeof(struct sockaddr_in)))
            return false;
        
        *len = sizeof(struct sockaddr_in);
        struct sockaddr_in *paddrin = reinterpret_cast<struct sockaddr_in*>(out);
        std::memset(paddrin, 0, *len);
        paddrin->sin_family = AF_INET;
        paddrin->sin_addr.s_addr = GetIpv4();
        paddrin->sin_port = htons(addr_.port());
        
        return true;
    }
    
    if (IsIpv6()) {
        if (*len < static_cast<socklen_t>(sizeof(struct sockaddr_in6)))
            return false;
        
        *len = sizeof(struct sockaddr_in6);        
        struct sockaddr_in6 *paddrin6 = reinterpret_cast<struct sockaddr_in6*>(out);
        std::memset(paddrin6, 0, *len);
        paddrin6->sin6_family = AF_INET6;
        GetIpv6(paddrin6->sin6_addr.s6_addr);
        paddrin6->sin6_port = htons(addr_.port());
        paddrin6->sin6_scope_id = addr_.scope_id();
        
        return true;
    }
    
    return false;
}

void NetAddr::Clear()
{
    addr_.clear_timestamp();
    addr_.clear_services();
    addr_.clear_ip();
    addr_.clear_port();
}

uint8_t NetAddr::GetByte(int n) const
{
    ASSERT_VALID_BYTE(n);
    const uint8_t *byte = reinterpret_cast<const uint8_t*>(addr_.ip().data());
    
    return byte[n];
}

void NetAddr::SetByte(int n, uint8_t value)
{
    ASSERT_VALID_BYTE(n);
    uint8_t *byte = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(addr_.ip().data()));
    byte[n] = value;
}

void NetAddr::SetNByte(const uint8_t *src, size_t n)
{
    ASSERT_VALID_BYTE(n);
    for (int i = 0; i < n; i++)
        SetByte(i, src[i]);
}

uint32_t NetAddr::GetIpv4() const
{
    ASSERT_SIZE();
    if (!IsIpv4())
        return INADDR_NONE;
    return addr_.ip(3);
}

void NetAddr::SetIpv4(uint32_t ip)
{
    ASSERT_SIZE();
    if (!IsIpv4())
        SetNByte(pch_ipv4, sizeof(pch_ipv4));
    addr_.set_ip(3, ip);
}

int NetAddr::GetIpv6(uint8_t *out) const
{
    ASSERT_SIZE();
    if (!IsIpv6())
        return -1;
    
    std::memcpy(out, addr_.ip().data(), ip_byte_size);
    
    return 0;
}

void NetAddr::SetIpv6(const uint8_t *src)
{
    ASSERT_SIZE();
    for (int i = 0; i < ip_byte_size; i++)
        SetByte(i, src[i]);
}


} // namespace btclite

SubNet::SubNet(const btclite::NetAddr& addr, int32_t mask)
    : net_addr_(addr), valid_(true)
{
    // Default to /32 (IPv4) or /128 (IPv6), i.e. match single address
    memset(netmask_, 0xff, sizeof(netmask_));

    // IPv4 addresses start at offset 12, and first 12 bytes must match, so just offset n
    const int astartofs = net_addr_.IsIpv4() ? 12 : 0;

    int32_t n = mask;
    if(n >= 0 && n <= (128 - astartofs*8)) // Only valid if in range of bits of address
    {
        n += astartofs*8;
        // Clear bits [n..127]
        for (; n < 128; ++n)
            netmask_[n>>3] &= ~(1<<(7-(n&7)));
    } else
        valid_ = false;

    // Normalize network according to netmask
    for(int x = 0; x < netmask_byte_size; ++x)
        net_addr_.SetByte(x, (net_addr_.GetByte(x) & netmask_[x]));
}

SubNet::SubNet(const btclite::NetAddr &addr, const btclite::NetAddr &mask)
    : net_addr_(addr), valid_(true)
{
    // Default to /32 (IPv4) or /128 (IPv6), i.e. match single address
    memset(netmask_, 255, sizeof(netmask_));

    // IPv4 addresses start at offset 12, and first 12 bytes must match, so just offset n
    const int astartofs = net_addr_.IsIpv4() ? 12 : 0;

    for(int x=astartofs; x<16; ++x)
        netmask_[x] = mask.GetByte(x);

    // Normalize network according to netmask
    for(int x = 0; x < netmask_byte_size; ++x)
        net_addr_.SetByte(x, (net_addr_.GetByte(x) & netmask_[x]));
}

bool SubNet::Match(const btclite::NetAddr& addr) const
{
    if (!valid_ || !addr.IsValid())
        return false;
    
    for (int i = 0; i < netmask_byte_size; ++i)
        if ((addr.GetByte(i) & netmask_[i]) != net_addr_.GetByte(i))
            return false;
    
    return true;
}

std::string SubNet::ToString() const
{
    return std::string();
}
