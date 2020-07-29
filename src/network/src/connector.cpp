#include "connector.h"

#include <arpa/inet.h>

#include "banlist.h"
#include "msg_process.h"
#include "net_base.h"
#include "peers.h"
#include "random.h"
#include "thread.h"


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
    if (nullptr == (bev_ = NewSocketEvent(Context()))) {
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

struct bufferevent *Connector::NewSocketEvent(const Context& ctx)
{
    struct bufferevent *bev;
    
    if (nullptr == (bev = bufferevent_socket_new(base_, -1, 
                              BEV_OPT_THREADSAFE | BEV_OPT_CLOSE_ON_FREE))) {
        BTCLOG(LOG_LEVEL_ERROR) << "Connector create socket event failed.";
        return nullptr;
    }
    
    bufferevent_setcb(bev, ConnReadCb, NULL, ConnEventCb, (void*)&ctx);
    bufferevent_enable(bev, EV_READ);
    
    return bev;
}

bool Connector::StartOutboundTimer(Context *ctx)
{    
    if (!ctx || !ctx->IsValid()) {
        BTCLOG(LOG_LEVEL_WARNING) << "Pass nullptr to " << __func__;
        return false;
    }
    
    if (outbounds_.ShouldDnsLookup()) {
        if (!ctx->ppeers->IsEmpty()) {
            auto func = std::bind(&Connector::DnsLookup, this, 
                                  std::ref(ctx->pparams->seeds()),
                                  ctx->pparams->default_port(), ctx->ppeers);
            util::SingletonTimerMng::GetInstance().StartTimer(11000, 0, 
                    std::function<bool()>(func));            
        }
        else {
            auto task = std::bind(&Connector::DnsLookup, this, 
                                  std::ref(ctx->pparams->seeds()),
                                  ctx->pparams->default_port(), ctx->ppeers);
            util::SingletonThreadPool::GetInstance().AddTask(
                std::function<bool()>(task));
        }
    }
    else {
        BTCLOG(LOG_LEVEL_INFO) << "P2P peers available. Skipped DNS seeding.";
    }
    
    ctx->pnodes = &outbounds_;
    auto func = std::bind(&Connector::OutboundTimeOutCb, this, std::ref(*ctx));
    outbound_timer_ = 
        util::SingletonTimerMng::GetInstance().StartTimer(500, 500, 
                std::function<bool()>(func));
    
    return (outbound_timer_ != nullptr);
}

bool Connector::OutboundTimeOutCb(const Context& ctx)
{
    proto_peers::Peer peer;
    NetAddr conn_addr;
    int tries;
    int64_t now;
    
    if (!ctx.IsValid()) {
        BTCLOG(LOG_LEVEL_WARNING) << "Pass nullptr to " << __func__;
        return false;
    }
    
    if (outbounds_.Size() >= kMaxOutboundConnections) {
        return false;
    }
    
    now = util::GetAdjustedTime();
    for (tries = 0; tries <= 100; ++tries) {
        if (util::SingletonInterruptor::GetInstance()) {
            return false;
        }
        
        if (!ctx.ppeers->Select(&peer)) {
            return false;
        }
        
        const NetAddr& addr = NetAddr(peer.addr());
        if (!addr.IsValid() || ctx.plocal_service->IsLocal(addr)) {
            return false;
        }
        
        if (tries <= 30) {
            if (now - peer.last_try() >= 600 &&
                peer.addr().port() == ctx.pparams->default_port() &&
                IsServiceFlagDesirable(peer.addr().services())) {
                return ConnectNode(addr, ctx);
            }
        }
        // consider very recently tried nodes after 30 failed attempts
        else if (tries <= 50) {
            if (peer.addr().port() == ctx.pparams->default_port() &&
                IsServiceFlagDesirable(peer.addr().services())) {
                return ConnectNode(addr, ctx);
            }
        }
        // allow non-default ports after 50 failed attempts
        else {
            if (IsServiceFlagDesirable(peer.addr().services())) {
                return ConnectNode(addr, ctx);
            }
        }
    }
    
    return false;
}

bool Connector::ConnectNodes(const std::vector<std::string>& str_addrs, 
                             const Context& ctx, bool manual)
{
    if (str_addrs.empty()) {
        return false;
    }
    
    if (!ctx.IsValid()) {
        BTCLOG(LOG_LEVEL_WARNING) << "Pass nullptr to " << __func__;
        return false;
    }
    
    std::vector<NetAddr> addrs;
    for (auto it = str_addrs.begin(); it != str_addrs.end(); ++it) {
        NetAddr addr;
        GetHostAddr(*it, ctx.pparams->default_port(), &addr);
        addrs.push_back(std::move(addr));
    }
    return ConnectNodes(addrs, ctx, manual);
}

bool Connector::ConnectNodes(const std::vector<NetAddr>& addrs, 
                             const Context& ctx, bool manual)
{
    bool ret = true;
    
    if (!ctx.IsValid()) {
        BTCLOG(LOG_LEVEL_WARNING) << "Pass nullptr to " << __func__;
        return false;
    }
    
    for (const NetAddr& addr : addrs) {
        ret &= ConnectNode(addr, ctx, manual);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return ret;
}

bool Connector::DnsLookup(const std::vector<Seed>& seeds, uint16_t port,
                          Peers *ppeers)
{
    int found = 0;
    
    if (!ppeers) {
        BTCLOG(LOG_LEVEL_WARNING) << "Pass nullptr to " << __func__;
        return false;
    }
    
    BTCLOG(LOG_LEVEL_INFO) << "Loading addresses from DNS seeds (could take a while)";

    for (const Seed& seed : seeds) {
        if (util::SingletonInterruptor::GetInstance())
            return false;

        std::string host = "x" + std::to_string(kDesirableServiceFlags) + "." + seed.host;

        NetAddr source;
        if (!source.SetInternal(host))
            continue;

        std::vector<NetAddr> addrs;
        // Limits number of addrs learned from a DNS seed
        if (!LookupHost(host.c_str(), &addrs, 256, true, port))
            continue;
        
        for (NetAddr& addr : addrs) {
            addr.set_port(port);
            addr.set_services(kDesirableServiceFlags);
            // use a random age between 3 and 7 days old
            int64_t time = util::GetTimeSeconds() - 3*24*60*60 
                           - util::RandUint64(4*24*60*60);
            addr.set_timestamp(time);
            found++;
        }
        ppeers->Add(addrs, source); 
    }
    
    if (!found) {
        BTCLOG(LOG_LEVEL_ERROR) << "No address found from DNS seeds. Maybe the network was down.";
        return false;
    }
    
    BTCLOG(LOG_LEVEL_INFO) << found << " addresses found from DNS seeds";        
    
    return true;
}

bool Connector::ConnectNode(const NetAddr& addr, const Context& ctx, bool manual)
{
    struct bufferevent *bev;
    struct sockaddr_storage sock_addr;
    socklen_t len;

    if (!base_) {
        return false;
    }
    
    if (!addr.IsValid()) {
        BTCLOG(LOG_LEVEL_WARNING) << "Connecting to invalide address:" << addr.ToString();
        return false;
    }

    if (ctx.plocal_service->IsLocal(addr)) {
        BTCLOG(LOG_LEVEL_WARNING) << "Connecting to local address:" << addr.ToString();
        return false;
    }

    // Look for an existing connection
    if (outbounds_.GetNode(addr)) {
        BTCLOG(LOG_LEVEL_WARNING) << "Failed to open new connection because it was already connected.";
        return false;
    }

    if (ctx.pbanlist->IsBanned(addr)) {
        BTCLOG(LOG_LEVEL_WARNING) << "Connecting to banned address:" << addr.ToString();
        return false;
    }

    if (nullptr == (bev = NewSocketEvent(ctx))) {
        return false;
    }

    BTCLOG(LOG_LEVEL_INFO) << "Trying to connect to " << addr.ToString() 
                           << ':' << addr.port();
    
    ctx.ppeers->Attempt(addr);

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

    auto node = outbounds_.InitializeNode(bev, addr, false, manual);
    if (!node) {
        BTCLOG(LOG_LEVEL_WARNING) << "Initialize new node failed.";
        bufferevent_free(bev);
        return false;
    }
    node->mutable_connection()->RegisterSetStateCb(NodeConnection::kDisconnected,
            std::bind(DisconnectNodeCb, node, &outbounds_, ctx.ppeers,
                      &SingletonBlocksInFlight::GetInstance(), &SingletonOrphans::GetInstance()));

    BTCLOG(LOG_LEVEL_INFO) << "Connected to " << addr.ToString() 
                              << ':' << addr.port();
    SendVersion(node, ctx.pparams->msg_magic(), ctx.pchain_state->ActiveChainHeight());
    
    return true;
}

bool Connector::GetHostAddr(const std::string& host_name, uint16_t port, NetAddr *out)
{
    std::vector<NetAddr> addrs;
    
    if (!LookupHost(host_name.c_str(), &addrs, 256, true, port) || 
            addrs.empty())
        return false;
    
    const NetAddr& addr = 
        addrs[util::RandUint64(addrs.size()-1)];
    if (!addr.IsValid()) {
        BTCLOG(LOG_LEVEL_WARNING) << "Invalide address " << addr.ToString() 
                                  << " for " << host_name;
        return false;
    }
    
    // It is possible that we already have a connection to the IP/port host_name resolved to.
    // In that case, drop the connection that was just created, and return the existing Node instead.
    // Also store the name we used to connect in that Node, so that future GetNode() calls to that
    // name catch this early.
    auto pnode = outbounds_.GetNode(addr);
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
