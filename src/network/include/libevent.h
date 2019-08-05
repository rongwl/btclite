#ifndef BTCLITE_LIBEVENT_H
#define BTCLITE_LIBEVENT_H


#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include "socket.h"


// for mock
class EventInterface {
public:
    virtual ~EventInterface() {}
    
    virtual bool EventBaseNew() = 0;
    virtual int EventBaseDispatch() = 0;
    virtual void EventBaseFree() = 0;
    virtual int EventBaseLoopexit(const struct timeval *tv) = 0;
    
    virtual struct bufferevent *BuffereventSocketNew(evutil_socket_t fd, int options) = 0;
    virtual void BuffereventSetcb(struct bufferevent *bev, bufferevent_data_cb readcb,
                                  bufferevent_data_cb writecb, bufferevent_event_cb eventcb, void *cbarg) = 0;
    virtual int BuffereventEnable(struct bufferevent *bev, short event) = 0;
    
    virtual bool EvconnlistenerNewBind(evconnlistener_cb cb, void *ptr, unsigned flags, int backlog,
                                      const struct sockaddr *sa, int socklen) = 0;
    virtual void EvconnlistenerSetErrorCb(evconnlistener_errorcb cb) = 0;
    virtual void EvconnlistenerFree() = 0;
};

class SockEvent : public EventInterface, Uncopyable {
public:
    SockEvent()
        : ev_base_(nullptr), ev_listener_(nullptr) {}
    
    //-------------------------------------------------------------------------
    bool EventBaseNew();
    int EventBaseDispatch();  
    void EventBaseFree();
    int EventBaseLoopexit(const struct timeval *tv);
    
    struct bufferevent *BuffereventSocketNew(evutil_socket_t fd, int options);
    void BuffereventSetcb(struct bufferevent *bev, bufferevent_data_cb readcb,
                          bufferevent_data_cb writecb, bufferevent_event_cb eventcb, void *cbarg);
    int BuffereventEnable(struct bufferevent *bev, short event);
    
    bool EvconnlistenerNewBind(evconnlistener_cb cb, void *ptr, unsigned flags, int backlog,
                              const struct sockaddr *sa, int socklen);    
    void EvconnlistenerSetErrorCb(evconnlistener_errorcb cb);   
    void EvconnlistenerFree();
    
    //-------------------------------------------------------------------------
    const struct event_base *ev_base() const
    {
        return ev_base_;
    }
    
    const struct evconnlistener *ev_listener() const
    {
        return ev_listener_;
    }
    
private:
    struct event_base *ev_base_;
    struct evconnlistener *ev_listener_;
};

#endif // BTCLITE_LIBEVENT_H
