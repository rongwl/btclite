#ifndef BTCLITE_CONNECTOR_H
#define BTCLITE_CONNECTOR_H


#include "libevent.h"
#include "node.h"


// outbound socket connection
class Connector {
public:
    Connector()
        : base_(nullptr), bev_(nullptr) {}
    
    ~Connector()
    {
        if (base_)
            event_base_free(base_);
    }
    
    bool InitEvent();
    void StartEventLoop();
    void ExitEventLoop()
    {
        struct timeval delay = {2, 0};
        BTCLOG(LOG_LEVEL_INFO) << "Exit acceptor event loop in 2s...";
        event_base_loopexit(base_, &delay);
    }
    struct bufferevent *NewSocketEvent();
    
    std::shared_ptr<Node> ConnectNode(const btclite::NetAddr& addr, const std::string& dst_name);
    
private:
    struct event_base *base_;
    struct bufferevent *bev_;
    
    static void ConnEventCb(struct bufferevent *bev, short events, void *ctx);
};

#endif //BTCLITE_CONNECTOR_H
