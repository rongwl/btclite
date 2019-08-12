#ifndef BTCLITE_ACCEPTOR_H
#define BTCLITE_ACCEPTOR_H


#include "libevent.h"
#include "node.h"


class Acceptor : Uncopyable {
public:
    Acceptor();
    
    /*~Acceptor()
    {
        sock_event_.EvconnlistenerFree();
        sock_event_.EventBaseFree();
    }*/
    
    bool StartEventLoop();
    void ExitEventLoop()
    {
        event_base_loopexit(base_, NULL);
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
    //static void 
};

#endif // BTCLITE_ACCEPTOR_H
