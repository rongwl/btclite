#include "connector.h"

#include "bandb.h"
#include "netbase.h"
#include "peers.h"
#include "random.h"
#include <arpa/inet.h>

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
    
    if (nullptr == (bev = bufferevent_socket_new(base_, -1, BEV_OPT_THREADSAFE | BEV_OPT_CLOSE_ON_FREE))) {
        BTCLOG(LOG_LEVEL_ERROR) << "Connector create socket event failed.";
        return nullptr;
    }
    
    bufferevent_setcb(bev, LibEvent::ConnReadCb, NULL, LibEvent::ConnEventCb, NULL);
    bufferevent_enable(bev, EV_READ);
    
    return bev;
}

bool Connector::StartOutboundTimer()
{
    auto func = std::bind(&Connector::OutboundTimeOutCb, this);
    outbound_timer_ = SingletonTimerMng::GetInstance().StartTimer(500, 500, std::function<bool()>(func));
    
    return (outbound_timer_ != nullptr);
}

bool Connector::OutboundTimeOutCb()
{
    proto_peers::Peer peer;
    btclite::NetAddr conn_addr;
    int tries;
    int64_t now = SingletonTime::GetInstance().GetAdjustedTime();
    
    tries = 0;
    while (tries <= 100) {
        if (SingletonNetInterrupt::GetInstance())
            return false;
        
        if (!SingletonPeers::GetInstance().Select(&peer))
            return false;
        
        const btclite::NetAddr& addr = btclite::NetAddr(peer.addr());
        if (!addr.IsValid() || SingletonLocalNetCfg::GetInstance().IsLocal(addr))
            return false;
        
        tries++;
        
        // only consider very recently tried nodes after 30 failed attempts
        if (now - peer.last_try() < 600 && tries < 30)
            continue;
        
        if (!HasAllDesirableServiceFlags(peer.addr().services()))
            continue;
        
        // do not allow non-default ports, unless after 50 invalid addresses selected already
        if (peer.addr().port() != Network::SingletonParams::GetInstance().default_port() && tries < 50)
            continue;
        
        conn_addr = std::move(addr);
        break;
    }

    if (!conn_addr.IsValid() || !ConnectNode(conn_addr))
        return false;

    if (SingletonNodes::GetInstance().CountOutbound() >= max_outbound_connections)
        outbound_timer_->Suspend();
    
    return true;
}

bool Connector::ConnectNodes(const std::vector<btclite::NetAddr>& addrs)
{
    bool ret = true;
    
    for (const btclite::NetAddr& addr : addrs) {
        ret &= ConnectNode(addr);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    return ret;
}

bool Connector::ConnectNode(const btclite::NetAddr& addr)
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

    BTCLOG(LOG_LEVEL_INFO) << "Trying to connect to " << addr.ToString() << ':' << addr.proto_addr().port();
    SingletonPeers::GetInstance().Attempt(addr);

    std::memset(&sock_addr, 0, sizeof(sock_addr));
    addr.ToSockAddr(reinterpret_cast<struct sockaddr*>(&sock_addr));
    len = (sock_addr.ss_family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);    
    if (bufferevent_socket_connect(bev, (struct sockaddr*)&sock_addr, len) < 0) {
        BTCLOG(LOG_LEVEL_WARNING) << "Connecting to " << addr.ToString() << " failed: " << strerror(errno);
        bufferevent_free(bev);
        return false;
    }

    auto node = SingletonNodes::GetInstance().InitializeNode(bev, addr, false);
    if (!node) {
        BTCLOG(LOG_LEVEL_WARNING) << "Initialize new node failed.";
        bufferevent_free(bev);
        return false;
    }
    
    return true;
}

bool Connector::GetHostAddr(const std::string& host_name, btclite::NetAddr *out)
{
    std::vector<btclite::NetAddr> addrs;
    
    if (!LookupHost(host_name.c_str(), &addrs, 256, true) || addrs.empty())
        return false;
    
    const btclite::NetAddr& addr = addrs[Random::GetUint64(addrs.size()-1)];
    if (!addr.IsValid()) {
        BTCLOG(LOG_LEVEL_WARNING) << "Invalide address " << addr.ToString() << " for " << host_name;
        return false;
    }
    
    // It is possible that we already have a connection to the IP/port host_name resolved to.
    // In that case, drop the connection that was just created, and return the existing Node instead.
    // Also store the name we used to connect in that Node, so that future GetNode() calls to that
    // name catch this early.
    auto pnode = SingletonNodes::GetInstance().GetNode(addr);
    if (pnode)
    {
        pnode->set_host_name(host_name);
        BTCLOG(LOG_LEVEL_WARNING) << "Failed to open new connection because it was already connected";
        return false;
    }
    
    *out = addr;
    
    return true;
}
