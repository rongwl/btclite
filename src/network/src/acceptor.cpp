#include "acceptor.h"

#include "bandb.h"
#include "block_sync.h"
#include "protocol/version.h"
#include "timer.h"


using namespace btclite::network::libevent;

Acceptor::Acceptor()
    : base_(nullptr), listener_(nullptr), sock_addr_()
{
    memset(&sock_addr_, 0, sizeof(sock_addr_));
    sock_addr_.sin6_family = AF_INET6;
    sock_addr_.sin6_port = htons(btclite::network::SingletonParams::GetInstance().default_port());
    sock_addr_.sin6_addr = in6addr_any;
    sock_addr_.sin6_scope_id = 0;
}

bool Acceptor::InitEvent()
{
    evthread_use_pthreads();
    
    if (nullptr == (base_ = event_base_new())) {
        BTCLOG(LOG_LEVEL_ERROR) << "Acceptor open event_base failed.";
        return false;
    }
    
    if (nullptr == (listener_ = evconnlistener_new_bind(base_, AcceptConnCb, NULL,
                                LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
                                SOMAXCONN, (const struct sockaddr*)&sock_addr_,
                                sizeof(sock_addr_))))
    {
        BTCLOG(LOG_LEVEL_ERROR) << "Acceptor create event listener failed.";
        return false;
    }
    
    //Disable Nagle's algorithm
    if (!Socket(evconnlistener_get_fd(listener_)).SetSockNoDelay()) {
        BTCLOG(LOG_LEVEL_ERROR) << "Acceptor setting socket to no-delay failed, error:"
                                << std::strerror(errno);
        return false;
    } 
    
    evconnlistener_set_error_cb(listener_, AcceptErrCb);
    
    return true;
}

void Acceptor::StartEventLoop()
{
    BTCLOG(LOG_LEVEL_INFO) << "Dispatching acceptor event loop...";

    if (base_) 
        event_base_dispatch(base_);
    else
        BTCLOG(LOG_LEVEL_ERROR) << "Event base for loop is null.";            
    
    BTCLOG(LOG_LEVEL_WARNING) << "Exited acceptor event loop.";
}

void Acceptor::AcceptConnCb(struct evconnlistener *listener, evutil_socket_t fd,
                           struct sockaddr *sock_addr, int socklen, void *arg)
{
    btclite::network::NetAddr addr;
    struct event_base *base;
    struct bufferevent *bev;
    struct event *ev_timeout;
    
    if (!addr.FromSockAddr(sock_addr))
        BTCLOG(LOG_LEVEL_WARNING) << "Accept an unknown socket family address.";
    
    // According to the internet TCP_NODELAY is not carried into accepted sockets
    // on all platforms.  Set it again here just to be sure.
    Socket(evconnlistener_get_fd(listener)).SetSockNoDelay();
    
    if (SingletonBanDb::GetInstance().IsBanned(addr))
    {
        BTCLOG(LOG_LEVEL_WARNING) << "Accept a dropped(banned) addr:" << addr.ToString();
        evutil_closesocket(fd);
        return;
    }
    
    if (SingletonNodes::GetInstance().CountInbound() >= kMaxInboundConnections) {
        BTCLOG(LOG_LEVEL_WARNING) << "Can not accept new connection because inbound connections is full";
        evutil_closesocket(fd);
        return;
    }
    
    if (nullptr == (base = evconnlistener_get_base(listener))) {
        BTCLOG(LOG_LEVEL_WARNING) << "Acceptor get event base by listener failed.";
        evutil_closesocket(fd);
        return;
    }
    
    if (nullptr == (bev = bufferevent_socket_new(base, fd, BEV_OPT_THREADSAFE | BEV_OPT_CLOSE_ON_FREE))) {
        BTCLOG(LOG_LEVEL_WARNING) << "Acceptor create buffer event for socker fd failed.";
        evutil_closesocket(fd);
        return;
    }
    
    if (nullptr == SingletonNodes::GetInstance().InitializeNode(bev, addr)) {
        BTCLOG(LOG_LEVEL_WARNING) << "Initialize new node failed.";
        bufferevent_free(bev);
        return;
    }
    
    bufferevent_setcb(bev, ConnReadCb, NULL, ConnEventCb, NULL);
    bufferevent_enable(bev, EV_READ);

    BTCLOG(LOG_LEVEL_VERBOSE) << "Accept connection from " << addr.ToString() << " successfully.";
}

void Acceptor::AcceptErrCb(struct evconnlistener *listener, void *arg)
{
    int err = EVUTIL_SOCKET_ERROR();

    BTCLOG(LOG_LEVEL_WARNING) << "Accept new socket failed, error:" 
                              << evutil_socket_error_to_string(err);
}

void Acceptor::CheckingTimeoutCb(evutil_socket_t fd, short event, void *arg)
{
    // increase reference count
    std::shared_ptr<Node> pnode(*(reinterpret_cast<std::shared_ptr<Node>*>(arg)));
    
    int64_t now = btclite::utility::util_time::GetTimeSeconds();
    if (pnode->time_last_recv() == 0 || pnode->time_last_send() == 0)
    {
        BTCLOG(LOG_LEVEL_INFO) << "socket no message in first 60 seconds, "
                               << pnode->time_last_recv() << " " << pnode->time_last_send()
                               << " from " << pnode->id();
        pnode->set_disconnected(true);
        SingletonNodes::GetInstance().EraseNode(pnode);
    }
    else if (now - pnode->time_last_send() > kConnTimeoutInterval)
    {
        BTCLOG(LOG_LEVEL_INFO) << "socket sending timeout: " << (now - pnode->time_last_send());
        pnode->set_disconnected(true);
        SingletonNodes::GetInstance().EraseNode(pnode);
    }
    else if (now - pnode->time_last_recv() > (pnode->version() > 
             btclite::network::protocol::kBip31Version ? kConnTimeoutInterval : 90*60))
    {
        BTCLOG(LOG_LEVEL_INFO) << "socket receive timeout: " << (now - pnode->time_last_recv());
        pnode->set_disconnected(true);
        SingletonNodes::GetInstance().EraseNode(pnode);
    }
    else if (pnode->ping_time().ping_nonce_sent &&
             pnode->ping_time().ping_usec_start + kConnTimeoutInterval * 1000000 < 
             btclite::utility::util_time::GetTimeMicros())
    {
        BTCLOG(LOG_LEVEL_INFO) << "ping timeout: " 
                               << (now - pnode->ping_time().ping_usec_start / 1000000);
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
