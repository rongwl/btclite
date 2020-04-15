#ifndef BTCLITE_CONNECTOR_H
#define BTCLITE_CONNECTOR_H


#include "libevent.h"
#include "node.h"


namespace btclite {
namespace network {

// outbound socket connection
class Connector {
public:
    Connector(const Params params)
        : params_(params), base_(nullptr), bev_(nullptr), 
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
    struct bufferevent *NewSocketEvent();
    
    //-------------------------------------------------------------------------
    bool StartOutboundTimer();
    bool OutboundTimeOutCb();
    bool ConnectNodes(const std::vector<std::string>& str_addrs, bool manual = false);
    bool ConnectNodes(const std::vector<NetAddr>& addrs, bool manual = false);
    bool GetHostAddr(const std::string& host_name, NetAddr *out);
    static bool DnsLookup(const std::vector<Seed>& seeds, uint16_t default_port);
    
private:
    const Params params_;
    
    struct event_base *base_;
    struct bufferevent *bev_;
    util::TimerMng::TimerPtr outbound_timer_;
    
    bool ConnectNode(const NetAddr& addr, bool manual = false);
};

} // namespace network
} // namespace btclite

#endif //BTCLITE_CONNECTOR_H
