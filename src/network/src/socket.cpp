#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "utility/include/logging.h"
#include "socket.h"


bool Socket::Create(int domain, int type, int protocol)
{
    if (sock_fd_ > 0) {
        BTCLOG(LOG_LEVEL_WARNING) << "socket already opened";
        return true;
    }
    
    if (-1 == (sock_fd_ = socket(domain, type, protocol))) {
        BTCLOG(LOG_LEVEL_ERROR) << "create socket failed, error:" << std::strerror(errno);
        return false;
    }
    
    //Disable Nagle's algorithm
    if (!SetSockNoDelay()) {
        Close();
        BTCLOG(LOG_LEVEL_ERROR) << "setting socket to no-delay failed, error:" << std::strerror(errno);
        return false;
    }
    
    // Set to non-blocking
    if (!SetSockNonBlocking()) {
        Close();
        BTCLOG(LOG_LEVEL_ERROR) << "setting socket to non-blocking failed, error:" << std::strerror(errno);
        return false;
    }
    
    return true;
}

bool Socket::Bind(const struct sockaddr *addr, socklen_t addr_len)
{
    if (-1 == bind(sock_fd_, addr, addr_len)) {
        BTCLOG(LOG_LEVEL_ERROR) << "binding addr to socket failed, error:" << std::strerror(errno);
        return false;
    }
    
    return true;
}

bool Socket::Listen(int back_log)
{
    if (-1 == listen(sock_fd_, SOMAXCONN)) {
        BTCLOG(LOG_LEVEL_ERROR) << "listening for incoming connections failed, error:" << std::strerror(errno);
        return false;
    }
    
    return true;
}

Socket::Fd Socket::Accept(struct sockaddr_storage *addr, socklen_t *addr_len)
{
    Fd conn_fd = accept(sock_fd_, (struct sockaddr*)addr, addr_len);
    if (conn_fd == -1) {
        BTCLOG(LOG_LEVEL_ERROR) << "accept new connection failed, error:" << std::strerror(errno);
    }
    
    return conn_fd;
}

bool Socket::Connect(const struct sockaddr *addr, socklen_t addr_len)
{
    if (-1 == connect(sock_fd_, addr, addr_len)) {
        BTCLOG(LOG_LEVEL_ERROR) << "socket connect failed, error:" << std::strerror(errno);
        return false;
    }
    
    return true;
}

bool Socket::Close()
{
    if (-1 == sock_fd_)
        return true;
    if (-1 == close(sock_fd_))
        return false;
    sock_fd_ = -1;
    
    return true;
}

bool Socket::SetSockNoDelay()
{
    int set = 1;
    if (-1 == setsockopt(sock_fd_, IPPROTO_TCP, TCP_NODELAY, (const char*)&set, sizeof(int)))
        return false;
    return true;
}

bool Socket::SetSockNonBlocking()
{
    int flags = fcntl(sock_fd_, F_GETFL, 0);
    if (-1 == fcntl(sock_fd_, F_SETFL, flags | O_NONBLOCK)) 
        return false;
    return true;
}

bool Socket::GetBindAddr(btclite::NetAddr *out)
{
    struct sockaddr_storage sockaddr_bind;
    socklen_t len = sizeof(sockaddr_bind);
    
    if (sock_fd_ > 0) {
        if (!getsockname(sock_fd_, (struct sockaddr*)&sockaddr_bind, &len))
            return out->FromSockAddr(reinterpret_cast<const struct sockaddr*>(&sockaddr_bind));
        else 
            BTCLOG_MOD(LOG_LEVEL_WARNING, Logging::NET) << "Warning: get bind addr failed";
    }
    
    return false;
}
