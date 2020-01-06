#ifndef BTCLITE_SOCKET_H
#define BTCLITE_SOCKET_H


#include "network/include/params.h"
#include "util.h"


namespace btclite {
namespace network {

// for mock
class SocketInterface {
public:
    virtual ~SocketInterface() {}
    
    virtual bool Create(int domain, int type, int protocol) = 0;
    virtual bool Bind(const struct sockaddr *addr, socklen_t addr_len) = 0;
    virtual bool Listen(int back_log) = 0;
    virtual int Accept(struct sockaddr_storage *addr, socklen_t *addr_len) = 0;
    virtual bool Connect(const struct sockaddr *addr, socklen_t addr_len) = 0;
    virtual bool Close() = 0;
    virtual int sock_fd() const = 0;
};

class Socket : public SocketInterface, util::Uncopyable {
public:
    using Fd = int;
    
    Socket()
        : sock_fd_(0) {}
    
    explicit Socket(Fd sock_fd)
        : sock_fd_(sock_fd) {}
    
    //-------------------------------------------------------------------------
    bool Create(int domain, int type, int protocol);
    bool Bind(const struct sockaddr *addr, socklen_t addr_len);
    bool Listen(int back_log);
    Fd Accept(struct sockaddr_storage *addr, socklen_t *addr_len);
    bool Connect(const struct sockaddr *addr, socklen_t addr_len);
    bool Close();    
    bool SetSockNoDelay();
    bool SetSockNonBlocking();
    bool GetBindAddr(NetAddr *out);
    
    //-------------------------------------------------------------------------
    Fd sock_fd() const
    {
        return sock_fd_;
    }
    
private:
    Socket::Fd sock_fd_;
};

class SingletonListenSocket : util::Uncopyable {
public:
    static Socket& GetInstance()
    {
        static Socket socket;
        return socket;
    }
    
private:
    SingletonListenSocket() {}
};

} // namespace network
} // namespace btclite

#endif // BTCLITE_SOCKET_H
