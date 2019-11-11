#ifndef BTCLITE_P2P_H
#define BTCLITE_P2P_H

#include <thread>

#include "acceptor.h"
#include "bandb.h"
#include "connector.h"
#include "net.h"
#include "network/include/params.h"
#include "peers.h"
#include "thread.h"


class P2P : Uncopyable {
public:
    explicit P2P(const ExecutorConfig& config);
    
    bool Init();
    bool Start();
    void Interrupt();
    void Stop();   
    
private:
    PeersDb peers_db_;
    Acceptor acceptor_;
    Connector connector_;
    
    std::thread thread_acceptor_loop_;
    std::thread thread_connector_loop_;
};

#endif // BTCLITE_P2P_H
