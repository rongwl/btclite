#include "acceptor.h"
#include "utility/include/logging.h"


bool Acceptor::Init(const Network::Params& params)
{
    bool ret = true, created = false;
    struct sockaddr_in sock_addr4;
    struct sockaddr_in6 sock_addr6;
    
    std::memset(&sock_addr4, 0, sizeof(sock_addr4));
    std::memset(&sock_addr6, 0, sizeof(sock_addr6));
    
    BasicSocket socket4(AF_INET);
    created = socket4.Create();
    if (created)    
        listen_sockets_.push_back(socket4);
    ret &= created;
    
    BasicSocket socket6(AF_INET6);
    created = socket6.Create();
    if (created)
        listen_sockets_.push_back(socket6);
    ret &= created;
    
    if (ret)
        BTCLOG(LOG_LEVEL_INFO) << "create all socket success";
    
    return ret;
}

bool Acceptor::Bind()
{
    bool ret = true;
    int one = 1;
    
    for (auto socket : listen_sockets_) {        
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
            ret &= false;
            continue;
        }
        else
            ret &= true;
    }
    
    if (ret)
        BTCLOG(LOG_LEVEL_INFO) << "bing all addrs to socket success";
    
    return ret;
}

bool Acceptor::Listen()
{
    bool ret = false;
    
    for (auto socket : listen_sockets_) {
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
