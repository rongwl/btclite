#ifndef BTCLITE_P2P_H
#define BTCLITE_P2P_H

#include <thread>

#include "acceptor.h"
#include "connector.h"
#include "network/include/params.h"


namespace btclite {
namespace network {

class P2P : util::Uncopyable {
public:
    explicit P2P(const util::Configuration& config)
        : params_(config), 
          peers_db_(config.path_data_dir()),
          ban_db_(config.path_data_dir()),
          acceptor_(params_.default_port()) {}
    
    bool Init(const chain::ChainState& chain_state);
    bool Start(const chain::ChainState& chain_state);
    void Interrupt();
    void Stop();   
    
private:
    const Params params_;
    LocalService local_service_;
    Peers peers_;
    PeersDb peers_db_;
    BanList ban_list_;
    BanDb ban_db_;
    Acceptor acceptor_;
    Connector connector_;
    //CollectionTimer collection_timer_;
    
    std::thread thread_acceptor_loop_;
    std::thread thread_connector_loop_;
};

} // namespace network
} // namespace btclite

#endif // BTCLITE_P2P_H
