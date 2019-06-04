#ifndef BTCLITE_SOCKET_H
#define BTCLITE_SOCKET_H


#include "network/include/params.h"


using Socket = int;

class BasicSocket {
public:
    BasicSocket()
        : sock_fd_(0), sock_addr_()
    {
        std::memset(&sock_addr_, 0, sizeof(sock_addr_));
    }
    
    BasicSocket(const struct sockaddr_in& sock_addr4)
    {
        std::memcpy(&sock_addr_, &sock_addr4, sizeof(sock_addr4));
    }
    
    BasicSocket(const struct sockaddr_in6& sock_addr6)
    {
        std::memcpy(&sock_addr_, &sock_addr6, sizeof(sock_addr6));
    }    
    
    //-------------------------------------------------------------------------
    bool Create();
    bool Close();    
    bool SetSockNoDelay();
    bool SetSockNonBlocking();
    
    //-------------------------------------------------------------------------
    Socket sock_fd() const
    {
        return sock_fd_;
    }
    
    const struct sockaddr_storage& sock_addr() const
    {
        return sock_addr_;
    }
    
private:
    Socket sock_fd_;
    struct sockaddr_storage sock_addr_;
};


#endif // BTCLITE_SOCKET_H
