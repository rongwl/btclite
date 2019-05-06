#include <arpa/inet.h>
#include <cstring>

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
    unsigned char ipNone6[16] = {};
    if (std::memcmp(addr_.ip().data(), ipNone6, 16) == 0)
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
    
    std::memcpy(out, addr_.ip().data(), 16);
    
    return 0;
}

void NetAddr::SetIpv6(const uint8_t *src)
{
    ASSERT_SIZE();
    for (int i = 0; i < 16; i++)
        SetByte(i, src[i]);
}


} // namespace btclite
