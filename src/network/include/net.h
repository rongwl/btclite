#ifndef BTCLITE_NET_H
#define BTCLITE_NET_H

#include <cstring>
#include <string>
#include <utility>

#include "network_address.pb.h"
#include "protocol.h"


class LocalNetConfig {
public:
	void LookupLocalAddrs();
	
	ServiceFlags local_services() const
	{
		return local_services_;
	}
	void set_local_services(ServiceFlags flags)
	{
		local_services_ = flags;
	}
	
	const std::vector<proto_netaddr::NetAddr>& local_addrs() const
	{
		return local_addrs_;
	}
	
private:
	ServiceFlags local_services_;
	std::vector<proto_netaddr::NetAddr> local_addrs_;
	
	bool AddLocalAddr();
};

#endif // BTCLITE_NET_H
