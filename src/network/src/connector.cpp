#include "connector.h"

#include "bandb.h"
#include "msg_process.h"
#include "net.h"
#include "netbase.h"
#include "peers.h"
#include "random.h"
#include <arpa/inet.h>


namespace btclite {
namespace network {

bool Connector::InitEvent()
{
    evthread_use_pthreads();
    
    if (nullptr == (base_ = event_base_new())) {
        BTCLOG(LOG_LEVEL_ERROR) << "Connector open event_base failed.";
        return false;
    }
    
    // default bufferevent, just for event loop keep running
    if (nullptr == (bev_ = NewSocketEvent())) {
        return false;
    }
    
    return true;
}

void Connector::StartEventLoop()
{    
    BTCLOG(LOG_LEVEL_INFO) << "Dispatching connector event loop...";
    
    if (base_) 
        event_base_dispatch(base_);
    else
        BTCLOG(LOG_LEVEL_ERROR) << "Event base for loop is null.";            
    
    BTCLOG(LOG_LEVEL_WARNING) << "Exited connector event loop.";
}

void Connector::ExitEventLoop()
{
    struct timeval delay = {2, 0};
    
    if (!base_)
        return;
    
    BTCLOG(LOG_LEVEL_INFO) << "Exit connector event loop in 2s...";
    event_base_loopexit(base_, &delay);
}

struct bufferevent *Connector::NewSocketEvent()
{
    struct bufferevent *bev;
    
    if (nullptr == (bev = bufferevent_socket_new(
                              base_, -1, BEV_OPT_THREADSAFE | BEV_OPT_CLOSE_ON_FREE))) {
        BTCLOG(LOG_LEVEL_ERROR) << "Connector create socket event failed.";
        return nullptr;
    }
    
    bufferevent_setcb(bev, ConnReadCb, NULL, ConnEventCb, NULL);
    bufferevent_enable(bev, EV_READ);
    
    return bev;
}

bool Connector::StartOutboundTimer()
{
    const std::vector<Seed>& seeds = 
        SingletonParams::GetInstance().seeds();
    if (!SingletonPeers::GetInstance().IsEmpty()) {
        auto func = std::bind(Connector::DnsLookup, std::placeholders::_1);
        util::SingletonTimerMng::GetInstance().StartTimer(11000, 0,
                std::function<bool(const std::vector<Seed>&)>(func), seeds);            
    }
    else {
        auto task = std::bind(Connector::DnsLookup, std::placeholders::_1);
        util::SingletonThreadPool::GetInstance().AddTask(
            std::function<bool(const std::vector<Seed>&)>(task), seeds);
    }
    
    auto func = std::bind(&Connector::OutboundTimeOutCb, this);
    outbound_timer_ = 
        util::SingletonTimerMng::GetInstance().StartTimer(500, 500, std::function<bool()>(func));
    
    return (outbound_timer_ != nullptr);
}

bool Connector::OutboundTimeOutCb()
{
    proto_peers::Peer peer;
    NetAddr conn_addr;
    int tries;
    int64_t now;
    
    if (SingletonNodes::GetInstance().CountOutbound() >= kMaxOutboundConnections)
        return false;
    
    now = util::GetAdjustedTime();
    tries = 0;
    while (tries <= 100) {
        if (SingletonNetInterrupt::GetInstance())
            return false;
        
        if (!SingletonPeers::GetInstance().Select(&peer))
            return false;
        
        const NetAddr& addr = NetAddr(peer.addr());
        if (!addr.IsValid() || SingletonLocalNetCfg::GetInstance().IsLocal(addr))
            return false;
        
        tries++;
        
        // only consider very recently tried nodes after 30 failed attempts
        if (now - peer.last_try() < 600 && tries < 30)
            continue;
        
        if (!IsServiceFlagDesirable(peer.addr().services()))
            continue;
        
        // do not allow non-default ports, unless after 50 invalid addresses selected already
        if (peer.addr().port() != 
                SingletonParams::GetInstance().default_port() && 
                tries < 50)
            continue;
        
        conn_addr = std::move(addr);
        break;
    }

    if (!conn_addr.IsValid() || !ConnectNode(conn_addr))
        return false;
    
    return true;
}

bool Connector::ConnectNodes(const std::vector<std::string>& str_addrs, bool manual)
{
    if (str_addrs.empty())
        return false;
    
    std::vector<NetAddr> addrs;
    for (auto it = str_addrs.begin(); it != str_addrs.end(); ++it) {
        NetAddr addr;
        GetHostAddr(*it, &addr);
        addrs.push_back(std::move(addr));
    }
    return ConnectNodes(addrs, manual);
}

bool Connector::ConnectNodes(const std::vector<NetAddr>& addrs, bool manual)
{
    bool ret = true;
    
    for (const NetAddr& addr : addrs) {
        ret &= ConnectNode(addr, manual);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    return ret;
}

bool Connector::DnsLookup(const std::vector<Seed>& seeds)
{
    int found = 0;
    
    if (!SingletonNodes::GetInstance().ShouldDnsLookup()) {
        BTCLOG(LOG_LEVEL_INFO) << "P2P peers available. Skipped DNS seeding.";
        return true;
    }
    
    BTCLOG(LOG_LEVEL_INFO) << "Loading addresses from DNS seeds (could take a while)";
    
    for (const Seed& seed : seeds) {
        if (SingletonNetInterrupt::GetInstance())
            return false;
        
        std::string host = "x" + std::to_string(kDesirableServiceFlags) + "." + seed.host;
        NetAddr source;
        if (!source.SetInternal(host))
            continue;
        
        std::vector<NetAddr> addrs;
        // Limits number of addrs learned from a DNS seed
        if (!LookupHost(host.c_str(), &addrs, 256, true))
            continue;
        
        for (NetAddr& addr : addrs) {
            addr.set_port(SingletonParams::GetInstance().default_port());
            addr.set_services(kDesirableServiceFlags);
            // use a random age between 3 and 7 days old
            int64_t time = util::GetTimeSeconds() - 3*24*60*60 
                           - util::GetUint64(4*24*60*60);
            addr.set_timestamp(time);
            found++;
        }
        SingletonPeers::GetInstance().Add(addrs, source); 
    }
    
    if (!found) {
        BTCLOG(LOG_LEVEL_ERROR) << "No address found from DNS seeds. Maybe the network was down.";
        return false;
    }
    
    BTCLOG(LOG_LEVEL_INFO) << found << " addresses found from DNS seeds";        
    
    return true;
}

bool Connector::ConnectNode(const NetAddr& addr, bool manual)
{
    struct bufferevent *bev;
    struct sockaddr_storage sock_addr;
    socklen_t len;

    if (!base_)
        return false;
    
    if (!addr.IsValid()) {
        BTCLOG(LOG_LEVEL_WARNING) << "Connecting to invalide address:" << addr.ToString();
        return false;
    }

    if (SingletonLocalNetCfg::GetInstance().IsLocal(addr)) {
        BTCLOG(LOG_LEVEL_WARNING) << "Connecting to local address:" << addr.ToString();
        return false;
    }

    // Look for an existing connection
    if (SingletonNodes::GetInstance().GetNode(addr))
    {
        BTCLOG(LOG_LEVEL_WARNING) << "Failed to open new connection because it was already connected.";
        return false;
    }

    if (SingletonBanDb::GetInstance().IsBanned(addr))
    {
        BTCLOG(LOG_LEVEL_WARNING) << "Connecting to banned address:" << addr.ToString();
        return false;
    }

    if (nullptr == (bev = NewSocketEvent())) {
        return false;
    }

    BTCLOG(LOG_LEVEL_INFO) << "Trying to connect to " << addr.ToString() 
                           << ':' << addr.port();
    SingletonPeers::GetInstance().Attempt(addr);

    std::memset(&sock_addr, 0, sizeof(sock_addr));
    addr.ToSockAddr(reinterpret_cast<struct sockaddr*>(&sock_addr));
    len = (sock_addr.ss_family == AF_INET) ? 
          sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);    
    if (bufferevent_socket_connect(bev, (struct sockaddr*)&sock_addr, len) < 0) {
        BTCLOG(LOG_LEVEL_WARNING) << "Connecting to " << addr.ToString()
                                  << " failed: " << strerror(errno);
        bufferevent_free(bev);
        return false;
    }

    auto node = SingletonNodes::GetInstance().InitializeNode(bev, addr, false, manual);
    if (!node) {
        BTCLOG(LOG_LEVEL_WARNING) << "Initialize new node failed.";
        bufferevent_free(bev);
        return false;
    }
    
    BTCLOG(LOG_LEVEL_VERBOSE) << "Connected to " << addr.ToString() 
                              << ':' << addr.port();
    
    SendVersion(node);
    
    return true;
}

bool Connector::GetHostAddr(const std::string& host_name, NetAddr *out)
{
    std::vector<NetAddr> addrs;
    
    if (!LookupHost(host_name.c_str(), &addrs, 256, true) || addrs.empty())
        return false;
    
    const NetAddr& addr = 
        addrs[util::GetUint64(addrs.size()-1)];
    if (!addr.IsValid()) {
        BTCLOG(LOG_LEVEL_WARNING) << "Invalide address " << addr.ToString() 
                                  << " for " << host_name;
        return false;
    }
    
    // It is possible that we already have a connection to the IP/port host_name resolved to.
    // In that case, drop the connection that was just created, and return the existing Node instead.
    // Also store the name we used to connect in that Node, so that future GetNode() calls to that
    // name catch this early.
    auto pnode = SingletonNodes::GetInstance().GetNode(addr);
    if (pnode)
    {
        pnode->mutable_connection()->set_host_name(host_name);
        BTCLOG(LOG_LEVEL_WARNING) << "Failed to open new connection because it was already connected";
        return false;
    }
    
    *out = addr;
    
    return true;
}

} // namespace network
} // namespace btclite
