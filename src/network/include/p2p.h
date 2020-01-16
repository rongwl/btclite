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


namespace btclite {
namespace network {

class P2P : util::Uncopyable {
public:
    explicit P2P(const util::ExecutorConfig& config);
    
    bool Init();
    bool Start();
    void Interrupt();
    void Stop();   
    
private:
    PeersDb peers_db_;
    Acceptor acceptor_;
    Connector connector_;
    CollectionTimer collection_timer_;
    
    std::thread thread_acceptor_loop_;
    std::thread thread_connector_loop_;
};

} // namespace network
} // namespace btclite

#endif // BTCLITE_P2P_H
