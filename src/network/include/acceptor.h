#ifndef BTCLITE_ACCEPTOR_H
#define BTCLITE_ACCEPTOR_H


#include "node.h"


// inbound socket connection
class Acceptor {
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

#endif // BTCLITE_ACCEPTOR_H
