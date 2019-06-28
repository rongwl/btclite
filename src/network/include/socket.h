#ifndef BTCLITE_SOCKET_H
#define BTCLITE_SOCKET_H


#include "network/include/params.h"


class Socket {
public:
    using Fd = int;
    
    Socket()
        : sock_fd_(0) {}
    
    explicit Socket(Socket::Fd sock_fd)
        : sock_fd_(sock_fd) {}
    
    //-------------------------------------------------------------------------
    bool Create();
    bool Close();    
    bool SetSockNoDelay();
    bool SetSockNonBlocking();
    bool GetBindAddr(btclite::NetAddr *out);
    
    //-------------------------------------------------------------------------
    Socket::Fd sock_fd() const
    {
        return sock_fd_;
    }
    
private:
    Socket::Fd sock_fd_;
};


#endif // BTCLITE_SOCKET_H
