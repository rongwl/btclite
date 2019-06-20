#ifndef BTCLITE_SOCKET_H
#define BTCLITE_SOCKET_H


#include "network/include/params.h"


using Socket = int;

class BasicSocket {
public:
    BasicSocket();
    explicit BasicSocket(sa_family_t family);
    
    //-------------------------------------------------------------------------
    bool Create();
    bool Close();    
    bool SetSockNoDelay();
    bool SetSockNonBlocking();
    static bool GetBindAddr(Socket sock_fd, btclite::NetAddr *out);
    
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
