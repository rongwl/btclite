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
#include "protocol/address.h"
#include "random.h"


namespace btclite {
namespace network {

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
            NetAddr addr;
            addr.SetIpv4(s4->sin_addr.s_addr);
            if (AddLocalHost(addr, kAcLocalIf))
                BTCLOG(LOG_LEVEL_INFO) << "Add local IPv4 addr " << ifa->ifa_name 
                                       << ":" << addr.ToString();
        }
        else if (ifa->ifa_addr->sa_family == AF_INET6)
        {
            struct sockaddr_in6* s6 = reinterpret_cast<struct sockaddr_in6*>(ifa->ifa_addr);
            NetAddr addr;
            addr.SetIpv6(s6->sin6_addr.s6_addr);
            if (AddLocalHost(addr, kAcLocalIf))
                BTCLOG(LOG_LEVEL_INFO) << "Add local IPv6 addr " << ifa->ifa_name 
                                       << ":" << addr.ToString();
        }
    }
    freeifaddrs(myaddrs);
    
    return !map_local_addrs_.empty();  
}

bool LocalNetConfig::GetLocalAddr(const NetAddr& peer_addr, ServiceFlags services,
                                  NetAddr *out)
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
        if (reachability > best_reachability || 
                (reachability == best_reachability && score > best_score))
        {
            *out = entry.first;
            best_reachability = reachability;
            best_score = score;
        }
    }
    
    if (best_score == -1)
        return false;
    
    out->set_services(services);
    out->set_timestamp(util::GetAdjustedTime());
    
    return true;
}

int LocalNetConfig::GetAddrScore(const NetAddr& peer_addr)
{
    LOCK(cs_local_net_config_);
    
    if (map_local_addrs_.count(peer_addr) == 0)
        return 0;
    
    return map_local_addrs_[peer_addr];
}

void LocalNetConfig::BroadcastTimeoutCb(std::shared_ptr<Node> node)
{
    if (SingletonNetInterrupt::GetInstance())
        return;
    
    if (node->connection().disconnected() || !node->connection().established()) 
        return;
    
    if (!IsInitialBlockDownload())
        return;
    
    AdvertiseLocalAddr(node);
    
    node->mutable_timers()->broadcast_local_addr_timer->set_interval(
        IntervalNextSend(kAvgLocalAddrBcInterval)*1000);
}

bool LocalNetConfig::AddLocalHost(const NetAddr& addr, int score)
{
    if (!addr.IsRoutable())
        return false;
    
    if (!SingletonNetArgs::GetInstance().should_discover() &&
            score < kAcManual)
        return false;

    LOCK(cs_local_net_config_);
    bool exist = map_local_addrs_.count(addr) > 0;
    if (!exist || score >= map_local_addrs_[addr]) {
        map_local_addrs_[addr] = score + (exist ? 1 : 0);
    }

    return true;
}

NetArgs::NetArgs(const util::Args& args)
    : listening_(args.GetBoolArg(FULLNODE_OPTION_LISTEN, true)),
      should_discover_(args.GetBoolArg(FULLNODE_OPTION_DISCOVER, true)),
      is_dnsseed_(args.GetBoolArg(FULLNODE_OPTION_DNSSEED, true)),
      specified_outgoing_()
{
    if (args.IsArgSet(FULLNODE_OPTION_CONNECT))
        specified_outgoing_ = args.GetArgs(FULLNODE_OPTION_CONNECT);
}


bool IsPeerLocalAddrGood(std::shared_ptr<Node> node)
{    
    return (SingletonNetArgs::GetInstance().should_discover() && 
            node->connection().addr().IsRoutable() && 
            node->connection().local_addr().IsRoutable());
}

void AdvertiseLocalAddr(std::shared_ptr<Node> node)
{    
    NetAddr addr_local;
    
    if (!SingletonNetArgs::GetInstance().listening() ||
        !node->connection().established())
        return;
    
    if (!SingletonLocalNetCfg::GetInstance().GetLocalAddr(
                node->connection().addr(), node->protocol().services(), &addr_local))
        return;
    
    int score = SingletonLocalNetCfg::GetInstance().GetAddrScore(addr_local);
    // If discovery is enabled, sometimes give our peer the address it
    // tells us that it sees us as in case it has a better idea of our
    // address than we do.
    if (IsPeerLocalAddrGood(node) && (!addr_local.IsRoutable() ||
                                       util::RandUint64(
                                           (score > kAcManual) ? 8:2) == 0))
        addr_local = std::move(node->connection().local_addr());
    
    if (addr_local.IsRoutable())
    {
        BTCLOG(LOG_LEVEL_INFO) << "Advertising local address " << addr_local.ToString();
        node->mutable_broadcast_addrs()->PushAddrToSend(addr_local);
    }
}

void BroadcastAddrsTimeoutCb(std::shared_ptr<Node> node)
{
    if (SingletonNetInterrupt::GetInstance())
        return;
    
    if (node->connection().disconnected() || !node->connection().established()) 
        return;
    
    std::vector<NetAddr> addrs_to_send = std::move(node->broadcast_addrs().addrs_to_send());
    protocol::Addr addr_msg;
    addr_msg.mutable_addr_list()->reserve(addrs_to_send.size());
    for (const NetAddr& addr : addrs_to_send) {
        if (!node->broadcast_addrs().IsKnownAddr(addr)) {
            node->mutable_broadcast_addrs()->AddKnownAddr(addr);
            addr_msg.mutable_addr_list()->push_back(addr);
            // receiver rejects addr messages larger than 1000
            if (addr_msg.addr_list().size() >= 1000) {
                SendMsg(addr_msg, node);
                addr_msg.mutable_addr_list()->clear();
            }
        }
    }
    if (!addr_msg.addr_list().empty())
        SendMsg(addr_msg, node);
    node->mutable_broadcast_addrs()->ClearSentAddr();
    
    node->mutable_timers()->broadcast_addr_timer->set_interval(
        IntervalNextSend(kAvgAddrBcInterval)*1000);
}


int64_t IntervalNextSend(int average_interval_seconds)
{
    long double log = log1p(util::RandUint64(1ULL << 48) * 
                       -0.0000000000000035527136788 /* -1/2^48 */);
    return static_cast<int64_t>(log * average_interval_seconds * -1000000.0 + 0.5);
}

void CollectionTimer::CheckDisconnectedNodes()
{
    std::vector<std::shared_ptr<Node> > disconnected_nodes;
    
    if (SingletonNetInterrupt::GetInstance())
        return;
    
    SingletonNodes::GetInstance().ClearDisconnected(&disconnected_nodes);
    for (auto& node : disconnected_nodes) {
        if (node->ShouldUpdateTime())                
            SingletonPeers::GetInstance().UpdateTime(node->connection().addr());
        
        for (auto& entry : node->blocks_in_flight().list()) {
            SingletonBlocksInFlight::GetInstance().Erase(entry.hash);
        }
        
        SingletonOrphans::GetInstance().EraseOrphansFor(node->id());
    }
}

} // namespace network
} // namespace btclite
