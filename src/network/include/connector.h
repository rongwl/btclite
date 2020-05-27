#ifndef BTCLITE_CONNECTOR_H
#define BTCLITE_CONNECTOR_H


#include "libevent.h"


namespace btclite {
namespace network {

// outbound socket connection
class Connector {
public:    
    Connector()
        : base_(nullptr), bev_(nullptr), 
          outbound_timer_(nullptr) {}
    
    ~Connector()
    {
        if (base_)
            event_base_free(base_);
    }    
    
    //-------------------------------------------------------------------------
    bool InitEvent();
    void StartEventLoop();
    void ExitEventLoop();
    struct bufferevent *NewSocketEvent(const Context& ctx);
    
    //-------------------------------------------------------------------------
    bool StartOutboundTimer(Context *ctx);
    bool OutboundTimeOutCb(const Context& ctx);
    bool ConnectNodes(const std::vector<std::string>& str_addrs, 
                      const Context& ctx, bool manual = false);
    bool ConnectNodes(const std::vector<NetAddr>& addrs, 
                      const Context& ctx, bool manual = false);
    bool GetHostAddr(const std::string& host_name, uint16_t port, NetAddr *out);
    bool DnsLookup(const std::vector<Seed>& seeds, uint16_t port,
                   Peers *ppeers);

    //-------------------------------------------------------------------------
    const Nodes& outbounds() const
    {
        return outbounds_;
    }
    
private:
    struct event_base *base_;
    struct bufferevent *bev_;
    util::TimerMng::TimerPtr outbound_timer_;
    
    Nodes outbounds_;
    
    bool ConnectNode(const NetAddr& addr, const Context& ctx, 
                     bool manual = false);
};

} // namespace network
} // namespace btclite

#endif //BTCLITE_CONNECTOR_H
