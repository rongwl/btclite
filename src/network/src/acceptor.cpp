#include "acceptor.h"
#include "bandb.h"
#include "block_sync.h"
#include "utiltime.h"


bool Acceptor::BindAndListen()
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
    if (!listen_socket_.Bind((const struct sockaddr*)&sock_addr6, sizeof(sock_addr6))) {
        listen_socket_.Close();
        return false;
    }
    
    if (!listen_socket_.Listen(SOMAXCONN)) {
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
    Socket::Fd conn_fd = listen_socket_.Accept(&sockaddr, &len);
    if (conn_fd == -1) {
        return false;
    }
    
    if (!addr.FromSockAddr(reinterpret_cast<const struct sockaddr*>(&sockaddr)))
        BTCLOG(LOG_LEVEL_WARNING) << "unknown socket family";
    std::cout << addr.ToString() << '\n';
    
    if (conn_fd >= FD_SETSIZE) {
        BTCLOG(LOG_LEVEL_WARNING) << "connection from " << addr.ToString() << " dropped: non-selectable socket";
        //close(conn_fd);
        return false;
    }
    
    // According to the internet TCP_NODELAY is not carried into accepted sockets
    // on all platforms.  Set it again here just to be sure.
    Socket(conn_fd).SetSockNoDelay();
    
    if (SingletonBanDb::GetInstance().IsBanned(addr))
    {
        BTCLOG_MOD(LOG_LEVEL_INFO, Logging::NET) << "connection from " << addr.ToString() << " dropped (banned)";
        //close(conn_fd);
        return false;
    }
    
    if (SingletonNodes::GetInstance().CountInbound() >= max_inbound_connections) {
        BTCLOG(LOG_LEVEL_INFO) << "can not accept new connection, inbound connections is full";
        //close(conn_fd);
        return false;
    }
    
    /*Node *node = new Node(conn_fd, addr, "", true);
    SingletonMapNodeState::GetInstance().Add(node->id(), node->addr(), node->host_name());
    SingletonNodes::GetInstance().Add(node);*/

    BTCLOG_MOD(LOG_LEVEL_DEBUG, Logging::NET) << "connection from " << addr.ToString() << " accepted";
    
    return true;
}

Accept2::Accept2()
    : sock_event_(), sock_addr_()
{
    memset(&sock_addr_, 0, sizeof(sock_addr_));
    sock_addr_.sin6_family = AF_INET6;
    sock_addr_.sin6_port = htons(Network::SingletonParams::GetInstance().default_port());
    sock_addr_.sin6_addr = in6addr_any;
    sock_addr_.sin6_scope_id = 0;
}

bool Accept2::StartEventLoop()
{
    if (!sock_event_.EventBaseNew())
        return false;
    
    if (!sock_event_.EvconnlistenerNewBind(AcceptConnCb, NULL, LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE,
                                          SOMAXCONN, (const struct sockaddr*)&sock_addr_, sizeof(sock_addr_)))
        return false;  
    
    sock_event_.EvconnlistenerSetErrorCb(AcceptErrCb);
    
    sock_event_.EventBaseDispatch();
    
    return true;
}

void Accept2::AcceptConnCb(struct evconnlistener *listener, evutil_socket_t fd,
                           struct sockaddr *sock_addr, int socklen, void *arg)
{
    btclite::NetAddr addr;
    struct event_base *base;
    struct bufferevent *bev;
    struct event *ev_timeout;
    
    if (!addr.FromSockAddr(sock_addr))
        BTCLOG(LOG_LEVEL_WARNING) << "unknown socket family";
    
    // According to the internet TCP_NODELAY is not carried into accepted sockets
    // on all platforms.  Set it again here just to be sure.
    Socket(evconnlistener_get_fd(listener)).SetSockNoDelay();
    
    if (SingletonBanDb::GetInstance().IsBanned(addr))
    {
        BTCLOG_MOD(LOG_LEVEL_INFO, Logging::NET) << "connection from " << addr.ToString() << " dropped (banned)";
        evutil_closesocket(fd);
        return;
    }
    
    if (SingletonNodes::GetInstance().CountInbound() >= max_inbound_connections) {
        BTCLOG(LOG_LEVEL_INFO) << "can not accept new connection, inbound connections is full";
        evutil_closesocket(fd);
        return;
    }
    
    if (nullptr == (base = evconnlistener_get_base(listener))) {
        BTCLOG(LOG_LEVEL_WARNING) << "get event base by listener failed";
        evutil_closesocket(fd);
        return;
    }
    
    if (nullptr == (bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE))) {
        BTCLOG(LOG_LEVEL_WARNING) << "create buffer event for socker fd failed";
        evutil_closesocket(fd);
        return;
    }
    
    auto node = std::make_shared<Node>(bev, addr);
    Nodes& nodes = SingletonNodes::GetInstance();
    nodes.AddNode(node);
    auto it = nodes.GetNode(node->id());
    if (it == nodes.End()) {
        BTCLOG(LOG_LEVEL_WARNING) << "save new node to nodes failed";
        bufferevent_free(bev);;
        return;
    }
    
    if (nullptr == (ev_timeout = event_new(base, fd, EV_PERSIST, CheckingTimeoutCb, (void*)(&(*it))))) {
        BTCLOG(LOG_LEVEL_WARNING) << "create timeout event for inactivity node checking failed";
        bufferevent_free(bev);;
        return;
    }
    
    if (event_add(ev_timeout, &node_checking_timeout) < 0) {
        BTCLOG(LOG_LEVEL_WARNING) << "add timeout event for inactivity node checking failed";
        bufferevent_free(bev);;
        return;
    }
    
    SingletonBlockSync::GetInstance().AddSyncState(node->id(), node->addr(), node->host_name());
    
    bufferevent_setcb(bev, ConnReadCb, NULL, ConnEventCb, (void*)(&(*it)));
    bufferevent_enable(bev, EV_READ);

    BTCLOG_MOD(LOG_LEVEL_DEBUG, Logging::NET) << "connection from " << addr.ToString() << " accepted";
}

void Accept2::AcceptErrCb(struct evconnlistener *listener, void *arg)
{
    struct event_base *base = evconnlistener_get_base(listener);
    int err = EVUTIL_SOCKET_ERROR();

    BTCLOG(LOG_LEVEL_WARNING) << "listen on event failed, error:" << evutil_socket_error_to_string(err);
}

void Accept2::ConnReadCb(struct bufferevent *bev, void *ctx)
{
    // increase reference count
    std::shared_ptr<Node> pnode(*(reinterpret_cast<std::shared_ptr<Node>*>(ctx)));
}

void Accept2::ConnEventCb(struct bufferevent *bev, short events, void *ctx)
{

}

void Accept2::CheckingTimeoutCb(evutil_socket_t fd, short event, void *arg)
{
    // increase reference count
    std::shared_ptr<Node> pnode(*(reinterpret_cast<std::shared_ptr<Node>*>(arg)));
    
    int64_t now = GetTimeSeconds();
    if (pnode->time_last_recv() == 0 || pnode->time_last_send() == 0)
    {
        BTCLOG(LOG_LEVEL_INFO) << "socket no message in first 60 seconds, "
                               << pnode->time_last_recv() << " " << pnode->time_last_send() << " from " << pnode->id();
        pnode->set_disconnected(true);
        SingletonNodes::GetInstance().EraseNode(pnode);
    }
    else if (now - pnode->time_last_send() > ping_timeout)
    {
        BTCLOG(LOG_LEVEL_INFO) << "socket sending timeout: " << (now - pnode->time_last_send());
        pnode->set_disconnected(true);
        SingletonNodes::GetInstance().EraseNode(pnode);
    }
    else if (now - pnode->time_last_recv() > (pnode->version() > bip0031_version ? ping_timeout : 90*60))
    {
        BTCLOG(LOG_LEVEL_INFO) << "socket receive timeout: " << (now - pnode->time_last_recv());
        pnode->set_disconnected(true);
        SingletonNodes::GetInstance().EraseNode(pnode);
    }
    else if (pnode->ping_time().ping_nonce_sent &&
             pnode->ping_time().ping_usec_start + ping_timeout * 1000000 < GetTimeMicros())
    {
        BTCLOG(LOG_LEVEL_INFO) << "ping timeout: " << (now - pnode->ping_time().ping_usec_start / 1000000);
        pnode->set_disconnected(true);
        SingletonNodes::GetInstance().EraseNode(pnode);
    }
    else if (!pnode->conn_established())
    {
        BTCLOG(LOG_LEVEL_INFO) << "version handshake timeout from " << pnode->id();
        pnode->set_disconnected(true);
        SingletonNodes::GetInstance().EraseNode(pnode);
    }
}
