#ifndef BTCLITE_LIBEVENT_H
#define BTCLITE_LIBEVENT_H


#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/thread.h>

#include "socket.h"


// for mock
class EventInterface {
public:
    virtual ~EventInterface() {}
    
    virtual struct event_base *EventBaseNew() = 0;
    virtual int EventBaseDispatch(struct event_base *event_base) = 0;
    virtual void EventBaseFree(struct event_base *base) = 0;
    virtual int EventBaseLoopexit(struct event_base *event_base, const struct timeval *tv) = 0;
    
    virtual struct bufferevent *BuffereventSocketNew(struct event_base *base, evutil_socket_t fd,
                                                     int options) = 0;
    virtual void BuffereventSetcb(struct bufferevent *bev, bufferevent_data_cb readcb,
                                  bufferevent_data_cb writecb, bufferevent_event_cb eventcb,
                                  void *cbarg) = 0;
    virtual int BuffereventEnable(struct bufferevent *bev, short event) = 0;
    
    virtual struct evconnlistener *EvconnlistenerNewBind(struct event_base *base, evconnlistener_cb cb,
                                                         void *ptr, unsigned flags, int backlog,
                                                         const struct sockaddr *sa, int socklen) = 0;
    virtual void EvconnlistenerSetErrorCb(struct evconnlistener *lev, evconnlistener_errorcb cb) = 0;
    virtual struct event_base *EvconnlistenerGetBase(struct evconnlistener *lev) = 0;
    virtual Socket::Fd EvconnlistenerGetFd(struct evconnlistener *lev) = 0;
    virtual void EvconnlistenerFree(struct evconnlistener *lev) = 0;
};

class LibEvent : public EventInterface, Uncopyable {
public:
    /*LibEvent()
        : ev_base_(nullptr), ev_listener_(nullptr) {}*/
    
    //-------------------------------------------------------------------------
    struct event_base *EventBaseNew();
    int EventBaseDispatch(struct event_base *event_base);  
    void EventBaseFree(struct event_base *base);
    int EventBaseLoopexit(struct event_base *event_base, const struct timeval *tv);
    
    struct bufferevent *BuffereventSocketNew(struct event_base *base, evutil_socket_t fd,
                                             int options);
    void BuffereventSetcb(struct bufferevent *bev, bufferevent_data_cb readcb,
                                 bufferevent_data_cb writecb, bufferevent_event_cb eventcb, void *cbarg);
    int BuffereventEnable(struct bufferevent *bev, short event);
    
    struct evconnlistener *EvconnlistenerNewBind(struct event_base *base, evconnlistener_cb cb,
                                                 void *ptr, unsigned flags, int backlog,
                                                 const struct sockaddr *sa, int socklen);    
    void EvconnlistenerSetErrorCb(struct evconnlistener *lev, evconnlistener_errorcb cb);
    struct event_base *EvconnlistenerGetBase(struct evconnlistener *lev);
    Socket::Fd EvconnlistenerGetFd(struct evconnlistener *lev);
    void EvconnlistenerFree(struct evconnlistener *lev);
    
    //-------------------------------------------------------------------------
    /*const struct event_base *ev_base() const
    {
        return ev_base_;
    }
    
    const struct evconnlistener *ev_listener() const
    {
        return ev_listener_;
    }*/
    
private:
    //struct event_base *ev_base_;
    //struct evconnlistener *ev_listener_;
};

#endif // BTCLITE_LIBEVENT_H
