#ifndef BTCLITE_ACCEPTOR_H
#define BTCLITE_ACCEPTOR_H


#include "libevent.h"


namespace btclite {
namespace network {

class Acceptor : util::Uncopyable {
public:    
    Acceptor(uint16_t listen_port);    
    ~Acceptor();
    
    bool InitEvent(Context *ctx);
    void StartEventLoop();    
    void ExitEventLoop();
    
    static void AcceptConnCb(struct evconnlistener *listener, evutil_socket_t fd,
                             struct sockaddr *addr, int socklen, void *arg);
    
    //-------------------------------------------------------------------------
    static Nodes& Inbounds();
    
    //-------------------------------------------------------------------------
    const struct sockaddr_in6& sock_addr() const;
    
private:
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
