#include "connector.h"
#include "netbase.h"
#include "random.h"


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

std::shared_ptr<Node> Connector::ConnectNode(const btclite::NetAddr& addr, const std::string& host_name)
{
    btclite::NetAddr host_addr;
    struct bufferevent *bev;
    
    if (host_name.empty()) {
        if (SingletonLocalNetCfg::GetInstance().IsLocal(addr))
            return nullptr;

        // Look for an existing connection
        if (SingletonNodes::GetInstance().GetNode(addr))
        {
            BTCLOG(LOG_LEVEL_WARNING) << "Failed to open new connection because it was already connected.";
            return nullptr;
        }
    }
    
    BTCLOG(LOG_LEVEL_INFO) << "Trying to connect to " << (host_name.empty() ? addr.ToString() : host_name);
    
    if (!host_name.empty()) {
        std::vector<btclite::NetAddr> addrs;
        if (LookupHost(host_name.c_str(), &addrs, 256, true) && !addrs.empty()) {
            host_addr = addrs[Random::GetUint64(addrs.size()-1)];
            if (!host_addr.IsValid()) {
                BTCLOG(LOG_LEVEL_WARNING) << "Invalide address " << host_addr.ToString() << " for " << host_name;
                return nullptr;
            }
            // It is possible that we already have a connection to the IP/port host_name resolved to.
            // In that case, drop the connection that was just created, and return the existing Node instead.
            // Also store the name we used to connect in that Node, so that future GetNode() calls to that
            // name catch this early.
            auto pnode = SingletonNodes::GetInstance().GetNode(host_addr);
            if (pnode)
            {
                pnode->set_host_name(host_name);
                BTCLOG(LOG_LEVEL_WARNING) << "Failed to open new connection because it was already connected";
                return nullptr;
            }
        }
    }
    
    if (nullptr == (bev = NewSocketEvent())) {
        return nullptr;
    }
    
    
    
    return nullptr;
}

void ConnEventCb(struct bufferevent *bev, short events, void *ctx)
{

}
