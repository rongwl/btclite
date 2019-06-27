#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "utility/include/logging.h"
#include "socket.h"


bool BasicSocket::Create()
{
    if (sock_fd_ > 0) {
        BTCLOG(LOG_LEVEL_WARNING) << "socket already opened";
        return true;
    }
    
    if (-1 == (sock_fd_ = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP))) {
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

bool BasicSocket::Close()
{
    if (-1 == sock_fd_)
        return true;
    if (-1 == close(sock_fd_))
        return false;
    sock_fd_ = -1;
    
    return true;
}

bool BasicSocket::SetSockNoDelay()
{
    int set = 1;
    if (-1 == setsockopt(sock_fd_, IPPROTO_TCP, TCP_NODELAY, (const char*)&set, sizeof(int)))
        return false;
    return true;
}

bool BasicSocket::SetSockNonBlocking()
{
    int flags = fcntl(sock_fd_, F_GETFL, 0);
    if (-1 == fcntl(sock_fd_, F_SETFL, flags | O_NONBLOCK)) 
        return false;
    return true;
}

bool BasicSocket::GetBindAddr(btclite::NetAddr *out)
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
