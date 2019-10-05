#include "net.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <ifaddrs.h>

#include "config.h"
#include "constants.h"
#include "network/include/params.h"


using namespace btclite::network;

bool LocalNetConfig::IsLocal(const btclite::network::NetAddr& addr)
{
    LOCK(cs_local_net_config_);
    for (auto it = local_addrs_.begin(); it != local_addrs_.end(); ++it)
        if (*it == addr)
            return true;
    
    return false;
}

bool LocalNetConfig::LookupLocalAddrs()
{
    // Get local host ip
    struct ifaddrs* myaddrs;
    
    if (getifaddrs(&myaddrs))
        return false;
    
    for (struct ifaddrs* ifa = myaddrs; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == nullptr)
            continue;
        if ((ifa->ifa_flags & IFF_UP) == 0)
            continue;
        if (strcmp(ifa->ifa_name, "lo") == 0)
            continue;
        if (strcmp(ifa->ifa_name, "lo0") == 0)
            continue;
        
        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            struct sockaddr_in* s4 = reinterpret_cast<struct sockaddr_in*>(ifa->ifa_addr);
            btclite::network::NetAddr addr;
            addr.SetIpv4(s4->sin_addr.s_addr);
            if (AddLocalHost(addr))
                BTCLOG(LOG_LEVEL_INFO) << "Add local IPv4 addr " << ifa->ifa_name << ":" << addr.ToString();
        }
        else if (ifa->ifa_addr->sa_family == AF_INET6)
        {
            struct sockaddr_in6* s6 = reinterpret_cast<struct sockaddr_in6*>(ifa->ifa_addr);
            btclite::network::NetAddr addr;
            addr.SetIpv6(s6->sin6_addr.s6_addr);
            if (AddLocalHost(addr))
                BTCLOG(LOG_LEVEL_INFO) << "Add local IPv6 addr " << ifa->ifa_name << ":" << addr.ToString();
        }
    }
    freeifaddrs(myaddrs);
    
    return !local_addrs_.empty();  
}

bool LocalNetConfig::AddLocalHost(const btclite::network::NetAddr& addr)
{
    if (!addr.IsRoutable())
        return false;

    LOCK(cs_local_net_config_);
    if (find(local_addrs_.begin(), local_addrs_.end(), addr) == local_addrs_.end())
    {
        local_addrs_.push_back(addr);
    }

    return true;
}

NetArgs::NetArgs(const Args& args)
    : is_listen_(args.GetBoolArg(FULLNODE_OPTION_LISTEN, true)),
      is_discover_(args.GetBoolArg(FULLNODE_OPTION_DISCOVER, true)),
      is_dnsseed_(args.GetBoolArg(FULLNODE_OPTION_DNSSEED, true)),
      specified_outgoing_()
{
    if (args.IsArgSet(FULLNODE_OPTION_CONNECT))
        specified_outgoing_ = args.GetArgs(FULLNODE_OPTION_CONNECT);
}
