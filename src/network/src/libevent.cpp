#include "libevent.h"


bool SockEvent::EventBaseNew()
{
    if (nullptr == (ev_base_ = event_base_new())) {
        BTCLOG(LOG_LEVEL_ERROR) << "open event_base failed";
        return false;
    }
        
    return true;
}

int SockEvent::EventBaseDispatch()
{
    if (ev_base_)
        return event_base_dispatch(ev_base_);
    return -1;
}

void SockEvent::EventBaseFree()
{
    if (ev_base_)
        event_base_free(ev_base_);
}

int SockEvent::EventBaseLoopexit(const struct timeval *tv)
{
    if (ev_base_)
        event_base_loopexit(ev_base_, tv);
}

struct bufferevent *SockEvent::BuffereventSocketNew(evutil_socket_t fd, int options)
{
    if (ev_base_)
        return bufferevent_socket_new(ev_base_, fd, options);
    return nullptr;
}

void SockEvent::BuffereventSetcb(struct bufferevent *bev, bufferevent_data_cb readcb,
                                 bufferevent_data_cb writecb, bufferevent_event_cb eventcb, void *cbarg)
{
    bufferevent_setcb(bev, readcb, writecb, eventcb, cbarg);
}

int SockEvent::BuffereventEnable(struct bufferevent *bev, short event)
{
    return bufferevent_enable(bev, event);
}

bool SockEvent::EvconnlistenerNewBind(evconnlistener_cb cb, void *ptr, unsigned flags, int backlog,
                                     const struct sockaddr *sa, int socklen)
{
    if (nullptr == (ev_listener_ = evconnlistener_new_bind(ev_base_, cb, ptr, flags, backlog, sa, socklen))) {
        EventBaseFree();
        BTCLOG(LOG_LEVEL_ERROR) << "create event listener failed";
        return false;
    }
    
    //Disable Nagle's algorithm
    if (!Socket(evconnlistener_get_fd(ev_listener_)).SetSockNoDelay()) {
        EvconnlistenerFree();
        EventBaseFree();
        BTCLOG(LOG_LEVEL_ERROR) << "setting socket to no-delay failed, error:" << std::strerror(errno);
        return false;
    }
    
    return true;
}

void SockEvent::EvconnlistenerSetErrorCb(evconnlistener_errorcb cb)
{
    if (ev_listener_)
        evconnlistener_set_error_cb(ev_listener_, cb);
}

void SockEvent::EvconnlistenerFree()
{
    if (ev_listener_)
        evconnlistener_free(ev_listener_);
}
