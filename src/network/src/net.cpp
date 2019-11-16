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
            if (AddLocalHost(addr, kAcLocalIf))
                BTCLOG(LOG_LEVEL_INFO) << "Add local IPv4 addr " << ifa->ifa_name << ":" << addr.ToString();
        }
        else if (ifa->ifa_addr->sa_family == AF_INET6)
        {
            struct sockaddr_in6* s6 = reinterpret_cast<struct sockaddr_in6*>(ifa->ifa_addr);
            btclite::network::NetAddr addr;
            addr.SetIpv6(s6->sin6_addr.s6_addr);
            if (AddLocalHost(addr, kAcLocalIf))
                BTCLOG(LOG_LEVEL_INFO) << "Add local IPv6 addr " << ifa->ifa_name << ":" << addr.ToString();
        }
    }
    freeifaddrs(myaddrs);
    
    return !map_local_addrs_.empty();  
}

bool LocalNetConfig::GetLocalAddr(const btclite::network::NetAddr& peer_addr,
                                  ServiceFlags services, btclite::network::NetAddr *out)
{
    int best_score = -1;
    int best_reachability = -1;
    
    if (!out)
        return false;
    
    if (!SingletonNetArgs::GetInstance().listening())
        return false;
    
    out->Clear();
    LOCK(cs_local_net_config_);
    for (const auto& entry : map_local_addrs_) {
        int score = entry.second;
        int reachability = entry.first.GetReachability(peer_addr);
        if (reachability > best_reachability || (reachability == best_reachability && score > best_score))
        {
            *out = entry.first;
            best_reachability = reachability;
            best_score = score;
        }
    }
    
    if (best_score == -1)
        return false;
    
    out->mutable_proto_addr()->set_services(services);
    out->mutable_proto_addr()->set_timestamp(btclite::utility::util_time::GetAdjustedTime());
    
    return true;
}

bool LocalNetConfig::AddLocalHost(const btclite::network::NetAddr& addr, int score)
{
    if (!addr.IsRoutable())
        return false;
    
    if (!SingletonNetArgs::GetInstance().should_discover() && score < kAcManual)
        return false;

    LOCK(cs_local_net_config_);
    bool exist = map_local_addrs_.count(addr) > 0;
    if (!exist || score >= map_local_addrs_[addr]) {
        map_local_addrs_[addr] = score + (exist ? 1 : 0);
    }

    return true;
}

NetArgs::NetArgs(const Args& args)
    : listening_(args.GetBoolArg(FULLNODE_OPTION_LISTEN, true)),
      should_discover_(args.GetBoolArg(FULLNODE_OPTION_DISCOVER, true)),
      is_dnsseed_(args.GetBoolArg(FULLNODE_OPTION_DNSSEED, true)),
      specified_outgoing_()
{
    if (args.IsArgSet(FULLNODE_OPTION_CONNECT))
        specified_outgoing_ = args.GetArgs(FULLNODE_OPTION_CONNECT);
}


namespace btclite {
namespace network {

bool IsPeerLocalAddrGood(std::shared_ptr<Node> node)
{    
    return (SingletonNetArgs::GetInstance().should_discover() && 
            node->addr().IsRoutable() && node->local_addr().IsRoutable());
}

} // namespace network
} // namespace btclite
