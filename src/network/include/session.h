#ifndef BTCLITE_SESSION_H
#define BTCLITE_SESSION_H

#include <deque>
#include <list>

#include "node.h"
#include "util.h"


class OutboundNode : public Node {
public:
private:
    //Connector connector_;
    std::deque<std::vector<unsigned char> > send_msg_;
};

class InboundNode: public Node {
public:
private:
    //Acceptor acceptor_;
    std::list<Message> recv_msg_;
};


class OutboundSession {
public:
    void GetAddrFromSeed();
    
private:
    std::unique_ptr<Semaphore> semaphore_;
    //OutboundNode node_;
    
    void OpenConnection();
}; 

class InboundSession {
public:
private:
    //InboundNode node_;
};

#endif // BTCLITE_SESSION_H
