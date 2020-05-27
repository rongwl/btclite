#include "net.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <math.h>

#include "block_sync.h"
#include "config.h"
#include "constants.h"
#include "msg_process.h"
#include "network/include/params.h"
#include "peers.h"
#include "protocol/addr.h"
#include "random.h"


namespace btclite {
namespace network {

bool LocalService::DiscoverLocalAddrs()
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
            NetAddr addr;
            addr.SetIpv4(s4->sin_addr.s_addr);
            if (AddLocalAddr(addr)) {
                BTCLOG(LOG_LEVEL_INFO) << "Add local IPv4 addr " << ifa->ifa_name 
                                       << ":" << addr.ToString();
            }
        }
        else if (ifa->ifa_addr->sa_family == AF_INET6)
        {
            struct sockaddr_in6* s6 = reinterpret_cast<struct sockaddr_in6*>(ifa->ifa_addr);
            NetAddr addr;
            addr.SetIpv6(s6->sin6_addr.s6_addr);
            if (AddLocalAddr(addr)) {
                BTCLOG(LOG_LEVEL_INFO) << "Add local IPv6 addr " << ifa->ifa_name 
                                       << ":" << addr.ToString();
            }
        }
    }
    freeifaddrs(myaddrs);
    
    return !local_addrs_.empty();  
}

bool LocalService::GetLocalAddr(const NetAddr& peer_addr, ServiceFlags services,
                                NetAddr *out) const
{
    int best_reachability = -1;
    
    if (!out)
        return false;
    
    out->Clear();
    LOCK(cs_local_service_);
    for (const auto& entry : local_addrs_) {
        int reachability = entry.GetReachability(peer_addr);
        if (reachability > best_reachability) {
            *out = entry;
            best_reachability = reachability;
        }
    }
    
    out->set_services(services);
    out->set_timestamp(util::GetAdjustedTime());
    
    return true;
}

void LocalService::AdvertiseLocalAddr(std::shared_ptr<Node> node, 
                                   bool discovered_addr_first) const
{    
    NetAddr addr_local;
    
    if (node->connection().IsDisconnected())
        return;
    
    if (!GetLocalAddr(node->connection().addr(), node->services(), &addr_local))
        return;
    
    if (discovered_addr_first && addr_local.IsRoutable()) {
        BTCLOG(LOG_LEVEL_INFO) << "Advertising local discovered address " 
                               << addr_local.ToString();
        node->mutable_flooding_addrs()->PushAddrToSend(addr_local);
    }
    // Sometimes give our peer the address it tells us that it sees us as in case 
    // it has a better idea of our address than we do.
    else if (node->connection().IsLocalAddrGood(true)) {
        BTCLOG(LOG_LEVEL_INFO) << "Advertising address(" << addr_local.ToString()
                               << ") that peer sees us";
        node->mutable_flooding_addrs()->PushAddrToSend(node->connection().local_addr());
    }
}

bool LocalService::IsLocal(const NetAddr& addr) const
{
    LOCK(cs_local_service_);
    
    for (const auto& entry : local_addrs_) {
        if (addr == entry)
            return true;
    }
    
    return false;
}

bool LocalService::AddLocalAddr(const NetAddr& addr)
{
    if (!addr.IsRoutable())
        return false;

    if (IsLocal(addr))
        return true;
    
    LOCK(cs_local_service_);
    local_addrs_.push_back(addr);

    return true;
}

void AdvertiseLocalTimeoutCb(std::shared_ptr<Node> node,
                             const LocalService& local_service)
{
    if (util::SingletonInterruptor::GetInstance())
        return;
    
    if (node->connection().IsDisconnected()) 
        return;
    
    if (IsInitialBlockDownload()) {
        node->mutable_timers()->advertise_local_addr_timer->set_interval(
                                IntervalNextSend(kAdvertiseLocalInterval)*1000);
        return;
    }
    
    local_service.AdvertiseLocalAddr(node, false);
    
    node->mutable_timers()->advertise_local_addr_timer->set_interval(
                            IntervalNextSend(kAdvertiseLocalInterval)*1000);
}

void RelayFloodingAddrsTimeoutCb(std::shared_ptr<Node> node, uint32_t magic)
{
    if (util::SingletonInterruptor::GetInstance())
        return;
    
    if (node->connection().IsDisconnected())
        return;
    
    std::vector<NetAddr>&& addrs_to_send = node->flooding_addrs().addrs_to_send();
    protocol::Addr addr_msg;
    addr_msg.mutable_addr_list()->reserve(addrs_to_send.size());
    for (const NetAddr& addr : addrs_to_send) {
        if (!node->flooding_addrs().IsKnownAddr(addr)) {
            node->mutable_flooding_addrs()->AddKnownAddr(addr);
            addr_msg.mutable_addr_list()->push_back(addr);
            // receiver rejects addr messages larger than 1000
            if (addr_msg.addr_list().size() >= 1000) {
                SendMsg(addr_msg, magic, node);
                addr_msg.mutable_addr_list()->clear();
            }
        }
    }
    if (!addr_msg.addr_list().empty())
        SendMsg(addr_msg, magic, node);
    node->mutable_flooding_addrs()->ClearSentAddr();
    
    node->mutable_timers()->broadcast_addrs_timer->set_interval(
        IntervalNextSend(kRelayAddrsInterval)*1000);
}


int64_t IntervalNextSend(int average_interval_seconds)
{
    long double log = log1p(util::RandUint64(1ULL << 48) * 
                       -0.0000000000000035527136788 /* -1/2^48 */);
    return static_cast<int64_t>(log * average_interval_seconds * -1000000.0 + 0.5);
}

void CollectionTimer::CheckDisconnectedNodes()
{

}

} // namespace network
} // namespace btclite
