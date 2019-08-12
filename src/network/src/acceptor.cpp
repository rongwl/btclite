#include "acceptor.h"
#include "bandb.h"
#include "block_sync.h"
#include "timer.h"


Acceptor::Acceptor()
    : base_(nullptr), listener_(nullptr), sock_addr_()
{
    memset(&sock_addr_, 0, sizeof(sock_addr_));
    sock_addr_.sin6_family = AF_INET6;
    sock_addr_.sin6_port = htons(Network::SingletonParams::GetInstance().default_port());
    sock_addr_.sin6_addr = in6addr_any;
    sock_addr_.sin6_scope_id = 0;
}

bool Acceptor::StartEventLoop()
{
    if (nullptr == (base_ = event_base_new())) {
        BTCLOG(LOG_LEVEL_ERROR) << "open event_base failed";
        return false;
    }
    
    if (nullptr == (listener_ = evconnlistener_new_bind(base_, AcceptConnCb, NULL,
                               LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE,
                               SOMAXCONN, (const struct sockaddr*)&sock_addr_,
                               sizeof(sock_addr_))))
    {
        event_base_free(base_);
        BTCLOG(LOG_LEVEL_ERROR) << "create event listener failed";
        return false;
    }
    
    //Disable Nagle's algorithm
    if (!Socket(evconnlistener_get_fd(listener_)).SetSockNoDelay()) {
        evconnlistener_free(listener_);
        event_base_free(base_);
        BTCLOG(LOG_LEVEL_ERROR) << "setting socket to no-delay failed, error:" << std::strerror(errno);
        return false;
    } 
    
    evconnlistener_set_error_cb(listener_, AcceptErrCb);
    
    event_base_dispatch(base_);
    
    return true;
}

void Acceptor::AcceptConnCb(struct evconnlistener *listener, evutil_socket_t fd,
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
    node->mutable_timers()->no_msg_timer = SingletonTimerMng::GetInstance().
                                           NewTimer(no_msg_timeout, 0, Node::InactivityTimeoutCb, node);
    
    Nodes& nodes = SingletonNodes::GetInstance();
    nodes.AddNode(node);    
    if (!nodes.GetNode(node->id())) {
        BTCLOG(LOG_LEVEL_WARNING) << "save new node to nodes failed";
        bufferevent_free(bev);
        return;
    }
    
    SingletonBlockSync::GetInstance().AddSyncState(node->id(), node->addr(), node->host_name());
    
    bufferevent_setcb(bev, ConnReadCb, NULL, ConnEventCb, NULL);
    bufferevent_enable(bev, EV_READ);

    BTCLOG_MOD(LOG_LEVEL_DEBUG, Logging::NET) << "connection from " << addr.ToString() << " accepted";
}

void Acceptor::AcceptErrCb(struct evconnlistener *listener, void *arg)
{
    struct event_base *base = evconnlistener_get_base(listener);
    int err = EVUTIL_SOCKET_ERROR();

    BTCLOG(LOG_LEVEL_WARNING) << "listen on event failed, error:" << evutil_socket_error_to_string(err);
}

void Acceptor::ConnReadCb(struct bufferevent *bev, void *ctx)
{
    // increase reference count
    std::shared_ptr<Node> pnode(SingletonNodes::GetInstance().GetNode(bev));
    assert(pnode != nullptr);
    
    if (pnode->timers().no_receiving_timer) {
        SingletonTimerMng::GetInstance().ResetTimer(pnode->timers().no_msg_timer);
    }
    
    if (pnode->timers().no_msg_timer) {
        TimerMng& timer_mng = SingletonTimerMng::GetInstance();
        timer_mng.StopTimer(pnode->timers().no_msg_timer);
        pnode->mutable_timers()->no_msg_timer.reset();
        
        uint32_t timeout = (pnode->version() > bip0031_version) ? no_receiving_timeout_bip31 : no_receiving_timeout;
        pnode->mutable_timers()->no_receiving_timer = timer_mng.NewTimer(timeout, 0, Node::InactivityTimeoutCb, pnode);
    }
    
    struct evbuffer *input = bufferevent_get_input(bev);    
    pnode->ParseMessage(input);
}

void Acceptor::ConnEventCb(struct bufferevent *bev, short events, void *ctx)
{
    // increase reference count
    std::shared_ptr<Node> pnode(SingletonNodes::GetInstance().GetNode(bev));
    assert(pnode != nullptr);
    
    if (events & BEV_EVENT_EOF) {
        if (!pnode->disconnected())
            BTCLOG(LOG_LEVEL_WARNING) << "peer " << pnode->id() << " socket closed";
        bufferevent_free(bev);
    }
    else if (events & BEV_EVENT_ERROR) {
        if (errno != EWOULDBLOCK && errno != EMSGSIZE && errno != EINTR && errno != EINPROGRESS)
        {
            if (!pnode->disconnected())
                BTCLOG(LOG_LEVEL_ERROR) << "peer " << pnode->id() << "socket recv error:"
                                        << std::string(strerror(errno));
            bufferevent_free(bev);
        }
    }
}

void Acceptor::CheckingTimeoutCb(evutil_socket_t fd, short event, void *arg)
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
    else if (now - pnode->time_last_send() > conn_timeout_interval)
    {
        BTCLOG(LOG_LEVEL_INFO) << "socket sending timeout: " << (now - pnode->time_last_send());
        pnode->set_disconnected(true);
        SingletonNodes::GetInstance().EraseNode(pnode);
    }
    else if (now - pnode->time_last_recv() > (pnode->version() > bip0031_version ? conn_timeout_interval : 90*60))
    {
        BTCLOG(LOG_LEVEL_INFO) << "socket receive timeout: " << (now - pnode->time_last_recv());
        pnode->set_disconnected(true);
        SingletonNodes::GetInstance().EraseNode(pnode);
    }
    else if (pnode->ping_time().ping_nonce_sent &&
             pnode->ping_time().ping_usec_start + conn_timeout_interval * 1000000 < GetTimeMicros())
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
