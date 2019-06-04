#ifndef BTCLITE_ACCEPTOR_H
#define BTCLITE_ACCEPTOR_H


#include "node.h"


// inbound socket connection
class Acceptor {
public:
    Acceptor()
        : listen_sockets_() {}
    
    bool Init(const Network::Params& params);
    bool Bind();
    bool Listen();
    Socket Accept();
    
    const std::vector<BasicSocket>& listen_sockets() const
    {
        return listen_sockets_;
    }
    
private:
    std::vector<BasicSocket> listen_sockets_;
};

#endif // BTCLITE_ACCEPTOR_H
