#include "acceptor.h"
#include "utility/include/logging.h"


bool Acceptor::Bind()
{
    struct sockaddr_in6 sock_addr6;
    int one = 1;
    
    // Allow binding if the port is still in TIME_WAIT state after
    // the program was closed and restarted.
    setsockopt(listen_socket_.sock_fd(), SOL_SOCKET, SO_REUSEADDR, (void*)&one, sizeof(int));

    std::memset(&sock_addr6, 0, sizeof(sock_addr6));
    sock_addr6.sin6_family = AF_INET6;
    std::memcpy(&sock_addr6.sin6_addr, &in6addr_any, sizeof(in6addr_any));
    sock_addr6.sin6_port = htons(Network::SingletonParams::GetInstance().default_port());
    sock_addr6.sin6_scope_id = 0;
    if (-1 == bind(listen_socket_.sock_fd(), (const struct sockaddr*)&sock_addr6, sizeof(sock_addr6))) {
        BTCLOG(LOG_LEVEL_ERROR) << "binding addr to socket failed, error:" << std::strerror(errno);
        listen_socket_.Close();
        return false;
    }
    
    return true;
}

bool Acceptor::Listen()
{
    if (-1 == listen(listen_socket_.sock_fd(), SOMAXCONN)) {
        BTCLOG(LOG_LEVEL_ERROR) << "listening for incoming connections failed, error:" << std::strerror(errno);
        listen_socket_.Close();
        return false;
    }
    
    return true;
}

bool Acceptor::Accept()
{
    struct sockaddr_storage sockaddr;
    socklen_t len = sizeof(sockaddr);
    btclite::NetAddr addr;
    
    std::memset(&sockaddr, 0, len);
    Socket sock_fd = accept(listen_socket_.sock_fd(), (struct sockaddr*)&sockaddr, &len);
    if (sock_fd == -1) {
        BTCLOG(LOG_LEVEL_ERROR) << "socket accept failed, error:" << std::strerror(errno);
        return false;
    }
    
    if (!addr.FromSockAddr(reinterpret_cast<const struct sockaddr*>(&sockaddr)))
        BTCLOG(LOG_LEVEL_WARNING) << "unknown socket family";
    
    if (sock_fd >= FD_SETSIZE) {
        BTCLOG(LOG_LEVEL_WARNING) << "connection from " << addr.ToString() << " dropped: non-selectable socket";
        close(sock_fd);
        return false;
    }
    
    // According to the internet TCP_NODELAY is not carried into accepted sockets
    // on all platforms.  Set it again here just to be sure.
    
    
    return true;
}
