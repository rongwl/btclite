#ifndef BTCLITE_NETWORK_ADDRESS_H
#define BTCLITE_NETWORK_ADDRESS_H

#include "network_address.pb.h"

namespace btclite {
	
class NetAddr {
public:
	NetAddr() = default;
	
	NetAddr(const proto_netaddr::NetAddr& addr)
		: addr_(addr) {}
	NetAddr(proto_netaddr::NetAddr&& addr) noexcept
		: addr_(std::move(addr)) {}
	
	NetAddr(const NetAddr& addr)
		: addr_(addr.addr_) {}
	NetAddr(NetAddr&& addr) noexcept
		: addr_(std::move(addr.addr_)) {}
	
	//-------------------------------------------------------------------------
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
	void Clear();
	
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
	
private:
	proto_netaddr::NetAddr addr_;
};

}

#endif // BTCLITE_NETWORK_ADDRESS_H
