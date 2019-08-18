#include "libevent.h"
#include "node.h"


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

void LibEvent::ConnReadCb(struct bufferevent *bev, void *ctx)
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

void LibEvent::ConnEventCb(struct bufferevent *bev, short events, void *ctx)
{
    // increase reference count
    std::shared_ptr<Node> pnode(SingletonNodes::GetInstance().GetNode(bev));
    assert(pnode != nullptr);
    
    if (events & BEV_EVENT_CONNECTED) {
    
    }
    else if (events & BEV_EVENT_EOF) {
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
