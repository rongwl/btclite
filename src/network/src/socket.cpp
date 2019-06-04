#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "utility/include/logging.h"
#include "socket.h"


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
