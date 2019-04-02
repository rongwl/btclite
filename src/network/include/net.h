#ifndef BTCLITE_NET_H
#define BTCLITE_NET_H

#include <cstring>
#include <string>
#include <utility>

#include "network_address.pb.h"
#include "protocol.h"


class LocalNetInfo {
public:
	void LookUpLocalHosts();
	
	ServiceFlags local_services() const
	{
		return local_services_;
	}
	void set_local_services(ServiceFlags flags)
	{
		local_services_ = flags;
	}
	
	const std::vector<proto_netaddr::NetAddr>& local_hosts() const
	{
		return local_hosts_;
	}
	
private:
	ServiceFlags local_services_;
	std::vector<proto_netaddr::NetAddr> local_hosts_;
	
	bool AddLocalHost();
};

#endif // BTCLITE_NET_H
