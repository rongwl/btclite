#ifndef BTCLITE_ACCEPTOR_H
#define BTCLITE_ACCEPTOR_H


#include "libevent.h"


namespace btclite {
namespace network {

class Acceptor : util::Uncopyable {
public:    
    Acceptor(const Params params);
    
    ~Acceptor()
    {
        if (listener_)
            evconnlistener_free(listener_);
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
    
    static void AcceptConnCb(struct evconnlistener *listener, evutil_socket_t fd,
                             struct sockaddr *addr, int socklen, void *arg);
    
    //-------------------------------------------------------------------------
    static Nodes& Inbounds()
    {
        static Nodes inbounds;
        return inbounds;
    }
    
    //-------------------------------------------------------------------------
    const struct sockaddr_in6& sock_addr() const
    {
        return sock_addr_;
    }
    
private:
    const Params params_;
    
    struct event_base *base_;
    struct evconnlistener *listener_;
    struct sockaddr_in6 sock_addr_;
    
    //-------------------------------------------------------------------------
    static void AcceptErrCb(struct evconnlistener *listener, void *arg);
    static void CheckingTimeoutCb(evutil_socket_t fd, short event, void *arg);
};

} // namespace network
} // namespace btclite

#endif // BTCLITE_ACCEPTOR_H
