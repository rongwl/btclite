#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "utility/include/logging.h"
#include "socket.h"


BasicSocket::BasicSocket()
    : sock_fd_(0), sock_addr_()
{
    struct sockaddr_in sock_addr4;
    
    std::memset(&sock_addr4, 0, sizeof(sock_addr4));
    std::memset(&sock_addr_, 0, sizeof(sock_addr_));
    sock_addr4.sin_family = AF_INET;
    sock_addr4.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_addr4.sin_port = htons(8333);
    std::memcpy(&sock_addr_, &sock_addr4, sizeof(sock_addr4));
}

BasicSocket::BasicSocket(sa_family_t family)
{
    std::memset(&sock_addr_, 0, sizeof(sock_addr_));
    
    if (family == AF_INET) {
        struct sockaddr_in sock_addr4;
        std::memset(&sock_addr4, 0, sizeof(sock_addr4));
        sock_addr4.sin_family = AF_INET;
        sock_addr4.sin_addr.s_addr = htonl(INADDR_ANY);
        sock_addr4.sin_port = htons(8333);
        std::memcpy(&sock_addr_, &sock_addr4, sizeof(sock_addr4));
    }
    else {
        struct sockaddr_in6 sock_addr6;
        std::memset(&sock_addr6, 0, sizeof(sock_addr6));
        sock_addr6.sin6_family = AF_INET6;
        std::memcpy(&sock_addr6.sin6_addr, &in6addr_any, sizeof(in6addr_any));
        sock_addr6.sin6_port = htons(8333);
        sock_addr6.sin6_scope_id = 0;
        std::memcpy(&sock_addr_, &sock_addr6, sizeof(sock_addr6));
    }
}

bool BasicSocket::Create()
{
    if (-1 == (sock_fd_ = socket(((struct sockaddr*)&sock_addr_)->sa_family, SOCK_STREAM, IPPROTO_TCP))) {
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

bool BasicSocket::GetBindAddr(Socket sock_fd, btclite::NetAddr *out)
{
    struct sockaddr_storage sockaddr_bind;
    socklen_t sockaddr_bind_len = sizeof(sockaddr_bind);
    
    if (sock_fd > 0) {
        if (!getsockname(sock_fd, (struct sockaddr*)&sockaddr_bind, &sockaddr_bind_len)) {
            out->FromSockAddr(reinterpret_cast<const struct sockaddr*>(&sockaddr_bind));
        } else {
            BTCLOG_MOD(LOG_LEVEL_INFO, Logging::NET) << "Warning: getsockname failed";
            return false;
        }
    }
    
    return true;
}
