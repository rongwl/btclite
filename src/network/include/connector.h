#ifndef BTCLITE_CONNECTOR_H
#define BTCLITE_CONNECTOR_H


#include "libevent.h"
#include "node.h"


// outbound socket connection
class Connector {
public:
    Connector()
        : base_(nullptr), bev_(nullptr), outbound_timer_(nullptr) {}
    
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
    bool ConnectNodes(const std::vector<btclite::NetAddr>& addrs);
    bool GetHostAddr(const std::string& host_name, btclite::NetAddr *out);
    
private:
    struct event_base *base_;
    struct bufferevent *bev_;
    TimerMng::TimerPtr outbound_timer_;
    
    bool ConnectNode(const btclite::NetAddr& addr);
    
};

#endif //BTCLITE_CONNECTOR_H
