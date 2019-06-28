#include "acceptor.h"
#include "bandb.h"
#include "random.h"
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
    Socket::Fd conn_fd = accept(listen_socket_.sock_fd(), (struct sockaddr*)&sockaddr, &len);
    if (conn_fd == -1) {
        BTCLOG(LOG_LEVEL_ERROR) << "socket accept failed, error:" << std::strerror(errno);
        return false;
    }
    
    if (!addr.FromSockAddr(reinterpret_cast<const struct sockaddr*>(&sockaddr)))
        BTCLOG(LOG_LEVEL_WARNING) << "unknown socket family";
    
    if (conn_fd >= FD_SETSIZE) {
        BTCLOG(LOG_LEVEL_WARNING) << "connection from " << addr.ToString() << " dropped: non-selectable socket";
        close(conn_fd);
        return false;
    }
    
    // According to the internet TCP_NODELAY is not carried into accepted sockets
    // on all platforms.  Set it again here just to be sure.
    Socket(conn_fd).SetSockNoDelay();
    
    if (SingletonBanDb::GetInstance().IsBanned(addr))
    {
        BTCLOG_MOD(LOG_LEVEL_INFO, Logging::NET) << "connection from " << addr.ToString() << " dropped (banned)";
        close(conn_fd);
        return false;
    }
    
    if (SingletonNodes::GetInstance().CountInbound() >= max_inbound_connections) {
        BTCLOG(LOG_LEVEL_INFO) << "can not accept new connection, inbound connections is full";
        close(conn_fd);
        return false;
    }
    
    btclite::NetAddr addr_bind;
    if (!Socket(conn_fd).GetBindAddr(&addr_bind)) {
        close(conn_fd);
        return false;
    }
    
    Node *node = new Node(SingletonNodes::GetInstance().GetNewNodeId(),
                          SingletonLocalNetCfg::GetInstance().local_services(),
                          SingletonBlockChain::GetInstance().Height(),
                          conn_fd, addr, Random::GetUint64(std::numeric_limits<uint64_t>::max()),
                          addr_bind, "", true);
    SingletonMapNodeState::GetInstance().Add(node->id(), node->addr(), node->host_name());
    
    return true;
}
