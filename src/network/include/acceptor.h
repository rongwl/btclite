#ifndef BTCLITE_ACCEPTOR_H
#define BTCLITE_ACCEPTOR_H


#include "libevent.h"
#include "node.h"


// inbound socket connection
class Acceptor : Uncopyable {
public:
    Acceptor()
        : listen_socket_(SingletonListenSocket::GetInstance())
    {
        listen_socket_.Create(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    }
    
    // for mock
    explicit Acceptor(SocketInterface& socket)
        : listen_socket_(socket) {}
    
    ~Acceptor()
    {
        listen_socket_.Close();
    }
    
    bool BindAndListen();
    bool Accept();

private:
    SocketInterface& listen_socket_;
};

class Accept2 : Uncopyable {
public:
    Accept2();
    
    ~Accept2()
    {
        sock_event_.EvconnlistenerFree();
        sock_event_.EventBaseFree();
    }
    
    bool StartEventLoop();
    
private:
    SockEvent sock_event_;
    struct sockaddr_in6 sock_addr_;
    
    static void AcceptConnCb(struct evconnlistener *listener, evutil_socket_t fd,
                             struct sockaddr *addr, int socklen, void *arg);
    static void AcceptErrCb(struct evconnlistener *listener, void *arg);
    static void ConnReadCb(struct bufferevent *bev, void *ctx);
    static void ConnEventCb(struct bufferevent *bev, short events, void *ctx);
    static void CheckingTimeoutCb(evutil_socket_t fd, short event, void *arg);
};

#endif // BTCLITE_ACCEPTOR_H
