#include "network_address.h"

#include <arpa/inet.h>
#include <botan/base32.h>
#include <sstream>

#include "constants.h"


namespace btclite {
namespace network {

NetAddr::NetAddr()
    : proto_addr_()
{
    proto_addr_.mutable_ip()->Resize(ip_uint32_size, 0);
}

NetAddr::NetAddr(const struct sockaddr_in& addr)
    : proto_addr_()
{
    proto_addr_.mutable_ip()->Resize(ip_uint32_size, 0);
    SetIpv4(addr.sin_addr.s_addr);
    proto_addr_.set_port(ntohs(addr.sin_port));
}

NetAddr::NetAddr(const struct sockaddr_in6& addr6)
    : proto_addr_()
{
    proto_addr_.mutable_ip()->Resize(ip_uint32_size, 0);
    SetIpv6(addr6.sin6_addr.s6_addr);
    proto_addr_.set_port(ntohs(addr6.sin6_port));
    proto_addr_.set_scope_id(addr6.sin6_scope_id);
}

NetAddr::NetAddr(const struct sockaddr_storage& addr)
    : proto_addr_()
{
    proto_addr_.mutable_ip()->Resize(ip_uint32_size, 0);
    if (addr.ss_family == AF_INET) {
        const struct sockaddr_in *addr4 = reinterpret_cast<const struct sockaddr_in*>(&addr);
        SetIpv4(addr4->sin_addr.s_addr);
        proto_addr_.set_port(ntohs(addr4->sin_port));
    }
    else {
        const struct sockaddr_in6 *addr6 = reinterpret_cast<const struct sockaddr_in6*>(&addr);
        SetIpv6(addr6->sin6_addr.s6_addr);
        proto_addr_.set_port(ntohs(addr6->sin6_port));
        proto_addr_.set_scope_id(addr6->sin6_scope_id);
    }
}

NetAddr::NetAddr(const proto_netaddr::NetAddr& proto_addr)
    : proto_addr_(proto_addr)
{
}

NetAddr::NetAddr(proto_netaddr::NetAddr&& proto_addr) noexcept
    : proto_addr_(std::move(proto_addr))
{
}


NetAddr::NetAddr(uint32_t timestamp, uint64_t services, IpAddr& ip, uint16_t port)
    : proto_addr_()
{
    proto_addr_.mutable_ip()->Resize(ip_uint32_size, 0);
    std::memcpy(proto_addr_.mutable_ip()->begin(), ip.begin(), kIpByteSize);
    proto_addr_.set_port(port);
    proto_addr_.set_services(services);
    proto_addr_.set_timestamp(timestamp);
}

NetAddr::NetAddr(uint32_t timestamp, uint64_t services, IpAddr&& ip, uint16_t port) noexcept
    : proto_addr_()
{
    proto_addr_.mutable_ip()->Resize(ip_uint32_size, 0);
    std::memmove(proto_addr_.mutable_ip()->begin(), ip.begin(), kIpByteSize);
    proto_addr_.set_port(port);
    proto_addr_.set_services(services);
    proto_addr_.set_timestamp(timestamp);
}

bool NetAddr::IsIpv4() const
{
    ASSERT_SIZE();
    return (std::memcmp(proto_addr_.ip().begin(), kPchIpv4, sizeof(kPchIpv4)) == 0);
}

bool NetAddr::IsIpv6() const
{
    ASSERT_SIZE();
    return (!IsIpv4() && !IsTor() && !IsInternal());
}

bool NetAddr::IsTor() const
{
    ASSERT_SIZE();
    return (std::memcmp(proto_addr_.ip().begin(), kPchOnionCat, sizeof(kPchOnionCat)) == 0);
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
    return IsIpv6() && (proto_addr_.ip(0) == 0x80FE && proto_addr_.ip(1) == 0);
}

bool NetAddr::IsRFC6052() const
{
    return IsIpv6() && (proto_addr_.ip(0) == 0x9BFF6400 && proto_addr_.ip(1) == 0);
}

bool NetAddr::IsRFC6145() const
{
    return IsIpv6() && (proto_addr_.ip(0) == 0 && proto_addr_.ip(1) == 0 && proto_addr_.ip(2)  == 0xFFFF);
}

bool NetAddr::IsLocal() const
{
    // IPv4 loopback
    if (IsIpv4() && (GetByte(12) == 127 || GetByte(12) == 0))
        return true;

    // IPv6 loopback (::1/128)
    if (IsIpv6() && (proto_addr_.ip(0) == 0 && proto_addr_.ip(1) == 0 && proto_addr_.ip(2) == 0 &&
                     proto_addr_.ip(3) == 0x1000000))
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
             (IsRFC4193() && !IsTor()) ||
             IsLocal() ||
             IsInternal());
}

bool NetAddr::IsInternal() const
{
    ASSERT_SIZE();
    return (std::memcmp(proto_addr_.ip().data(), kBtcIpPrefix, sizeof(kBtcIpPrefix)) == 0);
}

bool NetAddr::IsValid() const
{
    // Cleanup 3-byte shifted addresses caused by garbage in size field
    // of addr messages from versions before 0.2.9 checksum.
    // Two consecutive addr messages look like this:
    // header20 vectorlen3 addr26 addr26 addr26 header20 vectorlen3 addr26 addr26 addr26...
    // so if the first length field is garbled, it reads the second batch
    // of addr misaligned by 3 bytes.
    if (std::memcmp(proto_addr_.ip().data(), kPchIpv4+3, sizeof(kPchIpv4)-3) == 0)
        return false;

    // unspecified IPv6 address (::/128)
    unsigned char ip6_none[kIpByteSize] = {};
    if (std::memcmp(proto_addr_.ip().data(), ip6_none, kIpByteSize) == 0)
        return false;

    // documentation IPv6 address
    if (IsRFC3849())
        return false;

    if (IsInternal())
        return false;

    if (IsIpv4())
    {
        // INADDR_NONE
        if (proto_addr_.ip(3) == INADDR_NONE)
            return false;

        // 0
        if (proto_addr_.ip(3) == 0)
            return false;
    }
    
    return true;
}

std::string NetAddr::ToString() const
{
    std::stringstream ss;
    
    if (IsTor())
        return Botan::base32_encode(reinterpret_cast<const uint8_t*>(proto_addr_.ip().begin()) + sizeof(kPchOnionCat),
                                   kIpByteSize - sizeof(kPchOnionCat)) + ".onion";
    
    if (IsInternal())
        return Botan::base32_encode(reinterpret_cast<const uint8_t*>(proto_addr_.ip().begin()) + sizeof(kBtcIpPrefix),
                                    kIpByteSize - sizeof(kBtcIpPrefix)) + ".internal";

    if (IsIpv4())
        ss << +GetByte(12) << "." << +GetByte(13) << "." << +GetByte(14) << "." << +GetByte(15);
    else
        ss << std::hex << ((GetByte(0) << 8) | GetByte(1)) << ":" << ((GetByte(2) << 8) | GetByte(3)) << ":"
           << ((GetByte(4) << 8) | GetByte(5)) << ":" << ((GetByte(6) << 8) | GetByte(7)) << ":"
           << ((GetByte(8) << 8) | GetByte(9)) << ":" << ((GetByte(10) << 8) | GetByte(11)) << ":"
           << ((GetByte(12) << 8) | GetByte(13)) << ":" << ((GetByte(14) << 8) | GetByte(15));
    
    return ss.str();
}

bool NetAddr::ToSockAddr(struct sockaddr* out) const
{
    if (IsIpv4()) {
        struct sockaddr_in *addr = reinterpret_cast<struct sockaddr_in*>(out);
        std::memset(addr, 0, sizeof(*addr));
        addr->sin_family = AF_INET;
        addr->sin_addr.s_addr = GetIpv4();
        addr->sin_port = htons(proto_addr_.port());
        
        return true;
    }
    
    if (IsIpv6()) {
        struct sockaddr_in6 *addr6 = reinterpret_cast<struct sockaddr_in6*>(out);
        std::memset(addr6, 0, sizeof(*addr6));
        addr6->sin6_family = AF_INET6;
        GetIpv6(addr6->sin6_addr.s6_addr);
        addr6->sin6_port = htons(proto_addr_.port());
        addr6->sin6_scope_id = proto_addr_.scope_id();
        
        return true;
    }
    
    return false;
}

bool NetAddr::FromSockAddr(const struct sockaddr *in)
{
    if (in->sa_family == AF_INET) {
        *this = NetAddr(*reinterpret_cast<const struct sockaddr_in*>(in));
        return true;
    }
    else if (in->sa_family == AF_INET6) {
        *this = NetAddr(*reinterpret_cast<const sockaddr_in6*>(in));
        return true;
    }
    
    return false;
}

size_t NetAddr::SerializedSize() const
{
    return sizeof(proto_addr_.timestamp()) + sizeof(proto_addr_.services()) +
           kIpByteSize + sizeof(uint16_t);
}

void NetAddr::Clear()
{
    proto_addr_.clear_timestamp();
    proto_addr_.clear_services();
    for (int i = 0; i < proto_addr_.ip_size(); i++)
        proto_addr_.set_ip(i, 0);
    proto_addr_.clear_port();
}

uint8_t NetAddr::GetByte(int n) const
{
    ASSERT_VALID_BYTE(n);
    const uint8_t *byte = reinterpret_cast<const uint8_t*>(proto_addr_.ip().data());
    
    return byte[n];
}

void NetAddr::SetByte(int n, uint8_t value)
{
    ASSERT_VALID_BYTE(n);
    uint8_t *byte = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(proto_addr_.ip().data()));
    byte[n] = value;
}

void NetAddr::SetNByte(const uint8_t *src, size_t n)
{
    ASSERT_VALID_BYTE(n);
    for (int i = 0; i < n; i++)
        SetByte(i, src[i]);
}

// return ip in network byte order
uint32_t NetAddr::GetIpv4() const
{
    ASSERT_SIZE();
    if (!IsIpv4())
        return INADDR_NONE;
    return proto_addr_.ip(3);
}

void NetAddr::SetIpv4(uint32_t net_byte_order_ip)
{
    ASSERT_SIZE();
    if (!IsIpv4())
        SetNByte(kPchIpv4, sizeof(kPchIpv4));
    proto_addr_.set_ip(3, net_byte_order_ip);
}

bool NetAddr::GetIpv6(uint8_t *out) const
{
    ASSERT_SIZE();
    if (!IsIpv6())
        return false;
    
    std::memcpy(out, proto_addr_.ip().begin(), kIpByteSize);
    
    return true;
}

void NetAddr::SetIpv6(const uint8_t *src)
{
    ASSERT_SIZE();
    for (int i = 0; i < kIpByteSize; i++)
        SetByte(i, src[i]);
}

void NetAddr::GetGroup(std::vector<uint8_t> *out) const
{
    ASSERT_SIZE();
    int af = kAfIpv6;
    int start_byte = 0;
    int bits = 16;

    if (!out)
        return;
    
    out->clear();
    
    // all local addresses belong to the same group
    if (IsLocal())
    {
        af = 255;
        bits = 0;
    }
    // all internal-usage addresses get their own group
    if (IsInternal())
    {
        af = kAfInternal;
        start_byte = sizeof(kBtcIpPrefix);
        bits = (kIpByteSize - sizeof(kBtcIpPrefix)) * 8;
    }
    // all other unroutable addresses belong to the same group
    else if (!IsRoutable())
    {
        af = kAfUnroutable;
        bits = 0;
    }
    // for IPv4 addresses, '1' + the 16 higher-order bits of the IP
    // includes mapped IPv4, SIIT translated IPv4, and the well-known prefix
    else if (IsIpv4() || IsRFC6145() || IsRFC6052())
    {
        af = kAfIpv4;
        start_byte = 12;
    }
    // for 6to4 tunnelled addresses, use the encapsulated IPv4 address
    else if (IsRFC3964())
    {
        af = kAfIpv4;
        start_byte = 2;
    }
    // for Teredo-tunnelled IPv6 addresses, use the encapsulated IPv4 address
    else if (IsRFC4380())
    {
        out->push_back(kAfIpv4);
        out->push_back(GetByte(12) ^ 0xFF);
        out->push_back(GetByte(13) ^ 0xFF);
        return;
    }
    else if (IsTor())
    {
        af = kAfTor;
        start_byte = 6;
        bits = 4;
    }
    // for he.net, use /36 groups
    else if (GetByte(0) == 0x20 && GetByte(1) == 0x01 && GetByte(2) == 0x04 && GetByte(3) == 0x70)
        bits = 36;
    // for the rest of the IPv6 network, use /32 groups
    else
        bits = 32;

    out->push_back(af);
    while (bits >= 8)
    {
        out->push_back(GetByte(start_byte));
        start_byte++;
        bits -= 8;
    }
    if (bits > 0)
        out->push_back(GetByte(start_byte) | ((1 << (8 - bits)) - 1));
}

bool NetAddr::SetInternal(const std::string& name)
{
    if (name.empty()) {
        return false;
    }

    uint8_t hash[32] = {};
    std::unique_ptr<Botan::HashFunction> hash_func(Botan::HashFunction::create("SHA-256"));
    hash_func->update(reinterpret_cast<const uint8_t*>(name.data()), name.size());
    hash_func->final(hash);

    uint8_t *data = reinterpret_cast<uint8_t*>(proto_addr_.mutable_ip()->mutable_data());
    std::memcpy(data, kBtcIpPrefix, sizeof(kBtcIpPrefix));
    std::memcpy(data + sizeof(kBtcIpPrefix), hash, kIpByteSize - sizeof(kBtcIpPrefix));

    return true;
}

AddrFamily NetAddr::GetFamily() const
{
    if (IsInternal())
        return kAfInternal;

    if (!IsRoutable())
        return kAfUnroutable;

    if (IsIpv4())
        return kAfIpv4;

    if (IsTor())
        return kAfTor;

    return kAfIpv6;
}

int NetAddr::GetReachability(const NetAddr& addr_partner) const
{
    enum Reachability {
        kReachUnreachable,
        kReachDefault,
        kReachTeredo,
        kReachIpv6Weak,
        kReachIpv4,
        kReachIpv6Strong,
        kReachPrivate
    };

    if (!IsRoutable() || IsInternal())
        return kReachUnreachable;

    int our_net = GetExtFamily();
    int their_net = addr_partner.GetExtFamily();
    bool is_tunnel = IsRFC3964() || IsRFC6052() || IsRFC6145();

    switch(their_net) {
        case kAfIpv4:
            switch(our_net) {
                case kAfIpv4: 
                    return kReachIpv4;
                default:
                    return kReachDefault;                    
            }
        case kAfIpv6:
            switch(our_net) {
                case kAfTeredo:
                    return kReachTeredo;
                case kAfIpv4:
                    return kReachIpv4;
                case kAfIpv6:
                     // only prefer giving our IPv6 address if it's not tunnelled
                    return is_tunnel ? kReachIpv6Weak : kReachIpv6Strong;
                default:
                    return kReachDefault;                    
            }
        case kAfTor:
            switch(our_net) {
                case kAfIpv4:
                    return kReachIpv4; // Tor users can connect to IPv4 as well
                case kAfTor:
                    return kReachPrivate;
                default:
                    return kReachDefault;                    
            }
        case kAfTeredo:
            switch(our_net) {
                case kAfTeredo:
                    return kReachTeredo;
                case kAfIpv6:
                    return kReachIpv6Weak;
                case kAfIpv4:
                    return kReachIpv4;
                default:
                    return kReachDefault;                    
            }
        case kAfUnroutable:
        default:
            switch(our_net) {
                case kAfTeredo:
                    return kReachTeredo;
                case kAfIpv6:
                    return kReachIpv6Weak;
                case kAfIpv4:
                    return kReachIpv4;
                case kAfTor:
                    return kReachPrivate; // either from Tor, or don't care about our address
                default:
                    return kReachDefault;                    
            }
    }
}

AddrFamily NetAddr::GetExtFamily() const
{
    if (IsRFC4380())
        return kAfTeredo;

    return GetFamily();
}

util::Hash256 NetAddr::GetHash() const
{
    crypto::HashOStream hs;
    
    for (const auto& raw : proto_addr_.ip())
        hs << raw;
    hs << proto_addr_.port();
    
    return hs.Sha256();
}

uint16_t NetAddr::port() const
{
    return static_cast<uint16_t>(proto_addr_.port());
}

void NetAddr::set_port(uint16_t port)
{
    proto_addr_.set_port(port);
}

uint32_t NetAddr::scope_id() const
{
    return proto_addr_.scope_id();
}

void NetAddr::set_scope_id(uint32_t id)
{
    proto_addr_.set_scope_id(id);
}

uint64_t NetAddr::services() const
{
    return proto_addr_.services();
}

void NetAddr::set_services(uint64_t services)
{
    proto_addr_.set_services(services);
}

uint32_t NetAddr::timestamp() const
{
    return proto_addr_.timestamp();
}

void NetAddr::set_timestamp(uint32_t timestamp)
{
    proto_addr_.set_timestamp(timestamp);
}

const proto_netaddr::NetAddr& NetAddr::proto_addr() const
{
    return proto_addr_;
}

SubNet::SubNet()
    : net_addr_(), valid_(false)
{
    std::memset(netmask_, 0, sizeof(netmask_));
}

SubNet::SubNet(const NetAddr& addr)
    : net_addr_(addr), valid_(addr.IsValid())
{
    std::memset(netmask_, 0xff, sizeof(netmask_));
}

SubNet::SubNet(NetAddr&& addr) noexcept
    : net_addr_(std::move(addr)), valid_(addr.IsValid())
{
    std::memset(netmask_, 0xff, sizeof(netmask_));
}

SubNet::SubNet(const NetAddr& addr, int32_t mask)
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

SubNet::SubNet(const NetAddr &addr, const NetAddr &mask)
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

bool SubNet::IsValid() const
{
    return valid_;
}

void SubNet::Clear()
{
    net_addr_.Clear();
    std::memset(netmask_, 0, sizeof(netmask_));
    valid_ = false;
}

bool SubNet::Match(const NetAddr& addr) const
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
    /* Parse binary 1{n}0{N-n} to see if mask can be represented as /n */
    int cidr = 0;
    bool valid_cidr = true;
    int n = net_addr_.IsIpv4() ? 12 : 0;
    for (; n < 16 && netmask_[n] == 0xff; ++n)
        cidr += 8;
    if (n < 16) {
        int bits = NetmaskBits(netmask_[n]);
        if (bits < 0)
            valid_cidr = false;
        else
            cidr += bits;
        ++n;
    }
    for (; n < 16 && valid_cidr; ++n)
        if (netmask_[n] != 0x00)
            valid_cidr = false;

    /* Format output */
    std::stringstream os;
    if (valid_cidr) {
        os << cidr;
    } else {
        if (net_addr_.IsIpv4())
            os << +netmask_[12] << "." << +netmask_[13] << "." << +netmask_[14] << "." << +netmask_[15];
        else
            os << std::hex << (netmask_[0] << 8 | netmask_[1]) << ":" << (netmask_[2] << 8 | netmask_[3]) << ":"
               << (netmask_[4] << 8 | netmask_[5]) << ":" << (netmask_[6] << 8 | netmask_[7]) << ":"
               << (netmask_[8] << 8 | netmask_[9]) << ":" << (netmask_[10] << 8 | netmask_[11]) << ":"
               << (netmask_[12] << 8 | netmask_[13]) << ":" << (netmask_[14] << 8 | netmask_[15]);
    }

    return net_addr_.ToString() + "/" + os.str();
}

const NetAddr& SubNet::net_addr() const
{
    return net_addr_;
}

const uint8_t* const SubNet::netmask() const
{
    return netmask_;
}

int SubNet::NetmaskBits(uint8_t x) const
{
    switch(x) {
        case 0x00: 
            return 0;
        case 0x80: 
            return 1;
        case 0xc0:
            return 2;
        case 0xe0: 
            return 3;
        case 0xf0: 
            return 4;
        case 0xf8: 
            return 5;
        case 0xfc: 
            return 6;
        case 0xfe:
            return 7;
        case 0xff:
            return 8;
        default: 
            break;
    }
    
    return -1;
}

} // namespace network
} // namespace btclite
