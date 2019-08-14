#ifndef BTCLITE_ACCEPTOR_H
#define BTCLITE_ACCEPTOR_H


#include "libevent.h"
#include "node.h"


class Acceptor : Uncopyable {
public:
    Acceptor();
    
    ~Acceptor()
    {
        if (listener_)
            evconnlistener_free(listener_);
        if (base_)
            event_base_free(base_);
    }
    
    bool InitEvent();
    void StartEventLoop()
    {
        BTCLOG(LOG_LEVEL_INFO) << "Dispatching acceptor event loop...";

        if (base_) 
            event_base_dispatch(base_);
        else
            BTCLOG(LOG_LEVEL_ERROR) << "event base is null";            
        
        BTCLOG(LOG_LEVEL_WARNING) << "Exited acceptor event loop";
    }
    
    void ExitEventLoop()
    {
        struct timeval delay = {2, 0};
        BTCLOG(LOG_LEVEL_INFO) << "Exit acceptor event loop in 2s";
        event_base_loopexit(base_, &delay);
    }
    
    static void AcceptConnCb(struct evconnlistener *listener, evutil_socket_t fd,
                             struct sockaddr *addr, int socklen, void *arg);
    
    const struct sockaddr_in6& sock_addr() const
    {
        return sock_addr_;
    }
    
private:
    struct event_base *base_;
    struct evconnlistener *listener_;
    struct sockaddr_in6 sock_addr_;    
    
    static void AcceptErrCb(struct evconnlistener *listener, void *arg);
    static void ConnReadCb(struct bufferevent *bev, void *ctx);
    static void ConnEventCb(struct bufferevent *bev, short events, void *ctx);
    static void CheckingTimeoutCb(evutil_socket_t fd, short event, void *arg);
};

#endif // BTCLITE_ACCEPTOR_H
