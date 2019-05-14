#ifndef BTCLITE_SESSION_H
#define BTCLITE_SESSION_H

#include <deque>
#include <list>

//#include "net.h"
#include "socket.h"
#include "util.h"


/* Information about a connected peer */
class BaseNode {
public:
    using NodeId = int64_t;
    
    void Connect();
    void Disconnect();
    size_t Receive();
    size_t Send();
    
    NodeId id() const
    {
        return id_;
    }
private:
    NodeId id_;
};
// mixin uncopyable
using Node = Uncopyable<BaseNode>;

class OutboundNode : public Node {
public:
private:
    Connector connector_;
    std::deque<std::vector<unsigned char> > send_msg_;
};

class InboundNode: public Node {
public:
private:
    Acceptor acceptor_;
    std::list<Message> recv_msg_;
};


class OutboundSession {
public:
    void GetAddrFromSeed();
    
private:
    std::unique_ptr<Semaphore> semaphore_;
    OutboundNode nodes_;
    
    void OpenConnection();
}; 

class InboundSession {
public:
private:
    InboundNode nodes_;
};

#endif // BTCLITE_SESSION_H
