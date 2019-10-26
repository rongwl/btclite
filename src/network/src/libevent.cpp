#include "libevent.h"

#include "msg_process.h"
#include "protocol/version.h"


struct event_base *LibEvent::EventBaseNew()
{
    return event_base_new();
}

int LibEvent::EventBaseDispatch(struct event_base *event_base)
{
    if (event_base)
        return event_base_dispatch(event_base);
    return -1;
}

void LibEvent::EventBaseFree(struct event_base *base)
{
    if (base)
        event_base_free(base);
}

int LibEvent::EventBaseLoopexit(struct event_base *event_base, const struct timeval *tv)
{
    if (event_base)
        event_base_loopexit(event_base, tv);
}

struct bufferevent *LibEvent::BuffereventSocketNew(struct event_base *base, evutil_socket_t fd, int options)
{
    if (base)
        return bufferevent_socket_new(base, fd, options);
    return nullptr;
}

void LibEvent::BuffereventSetcb(struct bufferevent *bev, bufferevent_data_cb readcb,
                                 bufferevent_data_cb writecb, bufferevent_event_cb eventcb, void *cbarg)
{
    if (bev)
        bufferevent_setcb(bev, readcb, writecb, eventcb, cbarg);
}

int LibEvent::BuffereventEnable(struct bufferevent *bev, short event)
{
    if (bev)
        return bufferevent_enable(bev, event);
}

struct evconnlistener *LibEvent::EvconnlistenerNewBind(struct event_base *base, evconnlistener_cb cb,
                                                         void *ptr, unsigned flags, int backlog,
                                                         const struct sockaddr *sa, int socklen)
{
    if (base)
        return evconnlistener_new_bind(base, cb, ptr, flags, backlog, sa, socklen);
    
    return nullptr;
}

void LibEvent::EvconnlistenerSetErrorCb(struct evconnlistener *lev, evconnlistener_errorcb cb)
{
    if (lev)
        evconnlistener_set_error_cb(lev, cb);
}

Socket::Fd LibEvent::EvconnlistenerGetFd(struct evconnlistener *lev)
{
    if (lev)
        return evconnlistener_get_fd(lev);
}

struct event_base *LibEvent::EvconnlistenerGetBase(struct evconnlistener *lev)
{
    if (lev)
        return evconnlistener_get_base(lev);
}

void LibEvent::EvconnlistenerFree(struct evconnlistener *lev)
{
    if (lev)
        evconnlistener_free(lev);
}

namespace btclite {
namespace network {
namespace libevent {

void ConnReadCb(struct bufferevent *bev, void *ctx)
{
    // increase reference count
    std::shared_ptr<Node> pnode(SingletonNodes::GetInstance().GetNode(bev));
    if (!pnode)
        return;
    
    if (pnode->timers().no_receiving_timer) {
        pnode->mutable_timers()->no_receiving_timer->Reset();
    }
    
    if (pnode->timers().no_msg_timer) {
        TimerMng& timer_mng = SingletonTimerMng::GetInstance();
        timer_mng.StopTimer(pnode->timers().no_msg_timer);
        pnode->mutable_timers()->no_msg_timer.reset();
        
        uint32_t timeout = (pnode->version() >
                            btclite::network::protocol::kBip31Version) ? 
                            kNoReceivingTimeoutBip31 : kNoReceivingTimeout;
        pnode->mutable_timers()->no_receiving_timer = 
            timer_mng.StartTimer(timeout*1000, 0, Node::InactivityTimeoutCb, pnode);
    }
    
    auto task = std::bind(btclite::network::msg_process::ParseMsg, pnode);
    SingletonThreadPool::GetInstance().AddTask(std::function<bool()>(task));
}

void ConnEventCb(struct bufferevent *bev, short events, void *ctx)
{
    // increase reference count
    std::shared_ptr<Node> pnode(SingletonNodes::GetInstance().GetNode(bev));
    if (!pnode)
        return;
    
    if (events & BEV_EVENT_CONNECTED) {
        
    }
    else if (events & BEV_EVENT_EOF) {
        if (!pnode->disconnected())
            BTCLOG(LOG_LEVEL_WARNING) << "peer " << pnode->id() << " socket closed";
        pnode->set_conn_established(false);
        pnode->set_disconnected(true);
    }
    else if (events & BEV_EVENT_ERROR) {
        if (errno != EWOULDBLOCK && errno != EMSGSIZE && errno != EINTR && errno != EINPROGRESS)
        {
            if (!pnode->disconnected())
                BTCLOG(LOG_LEVEL_ERROR) << "peer " << pnode->id() << " socket recv error:"
                                        << std::string(strerror(errno));
            pnode->set_conn_established(false);
            pnode->set_disconnected(true);
        }
    }
}

} // namespace libevent
} // namespace network
} // namespace btclite
