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

bool Connector::Connect()
{
    return true;
}

bool Acceptor::Init(const Network::Params& params)
{
    bool ret = false, created = false;
    struct sockaddr_in sock_addr4;
    struct sockaddr_in6 sock_addr6;
    
    memset(&sock_addr4, 0, sizeof(sock_addr4));
    memset(&sock_addr6, 0, sizeof(sock_addr6));
    
    sock_addr4.sin_family = AF_INET;
    sock_addr4.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_addr4.sin_port = htons(params.default_port());
    BasicSocket socket4(sock_addr4);
    created = socket4.Create();
    if (created)    
        sockets_.push_back(socket4);
    ret |= created;
    
    sock_addr6.sin6_family = AF_INET6;
    std::memcpy(&sock_addr6.sin6_addr, &in6addr_any, sizeof(in6addr_any));
    sock_addr6.sin6_port = htons(params.default_port());
    sock_addr6.sin6_scope_id = 0;
    BasicSocket socket6(sock_addr6);
    created = socket6.Create();
    if (created)
        sockets_.push_back(socket6);
    ret |= created;
    
    if (ret)
        BTCLOG(LOG_LEVEL_INFO) << "create all socket success";
    
    return ret;
}

bool Acceptor::Bind()
{
    bool ret = false;
    int one = 1;
    
    for (auto socket : sockets_) {        
        // Allow binding if the port is still in TIME_WAIT state after
        // the program was closed and restarted.
        setsockopt(socket.sock_fd(), SOL_SOCKET, SO_REUSEADDR, (void*)&one, sizeof(int));
        
        // some systems don't have IPV6_V6ONLY but are always v6only; others do have the option
        // and enable it by default or not. Try to enable it, if possible.
        if (socket.sock_addr().ss_family == AF_INET6)
            setsockopt(socket.sock_fd(), IPPROTO_IPV6, IPV6_V6ONLY, (void*)&one, sizeof(int));

        if (-1 == bind(socket.sock_fd(), (const struct sockaddr*)&socket.sock_addr(), sizeof(socket.sock_addr()))) {
            BTCLOG(LOG_LEVEL_ERROR) << "binding addr to socket failed, error:" << std::strerror(errno);
            socket.Close();
            ret |= false;
            continue;
        }
        else
            ret |= true;
    }
    
    if (ret)
        BTCLOG(LOG_LEVEL_INFO) << "bing all addrs to socket success";
            
    return ret;
}

bool Acceptor::Listen()
{
    bool ret = false;
    
    for (auto socket : sockets_) {
        if (-1 == listen(socket.sock_fd(), SOMAXCONN)) {
            BTCLOG(LOG_LEVEL_ERROR) << "listening for incoming connections failed, error:" << std::strerror(errno);
            socket.Close();
            ret |= false;
            continue;
        }
        else
            ret |= true;
    }
    
    if (ret)
        BTCLOG(LOG_LEVEL_INFO) << "ready to listen for all incomming connections";
    
    return ret;
}

Socket Acceptor::Accept()
{
    return 0;
}
