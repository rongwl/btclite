#ifndef BTCLITE_ACCEPTOR_H
#define BTCLITE_ACCEPTOR_H


#include "node.h"


// inbound socket connection
class Acceptor {
public:
    Acceptor()
        : listen_socket_()
    {
        listen_socket_.Create();
    }
    
    ~Acceptor()
    {
        listen_socket_.Close();
    }
    
    bool Bind();
    bool Listen();
    bool Accept();
    
    const BasicSocket& listen_socket() const
    {
        return listen_socket_;
    }
    
private:
    BasicSocket listen_socket_;
};

#endif // BTCLITE_ACCEPTOR_H
