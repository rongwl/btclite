#ifndef BTCLITE_P2P_H
#define BTCLITE_P2P_H

#include <thread>

#include "acceptor.h"
#include "bandb.h"
#include "connector.h"
#include "network/include/params.h"
#include "peers.h"
#include "thread.h"


class P2P : Uncopyable {
public:
    explicit P2P(const ExecutorConfig& config)
        : network_params_(Network::SingletonParams::GetInstance(config.env())),
          network_args_(config.args()), local_network_config_(SingletonLocalNetCfg::GetInstance()), 
          nodes_(SingletonNodes::GetInstance()),
          ban_db_(SingletonBanDb::GetInstance(config.path_data_dir())),
          peers_db_(config.path_data_dir()), acceptor_(), connector_(),
          interrupt_(SingletonNetInterrupt::GetInstance()) {}
    
    bool Init();
    bool Start();
    void Interrupt();
    void Stop();
    
    //-------------------------------------------------------------------------    
    const NetArgs& network_args() const
    {
        return network_args_;
    }
    
private:
    Network::Params& network_params_;
    NetArgs network_args_;
    LocalNetConfig& local_network_config_;
    Nodes& nodes_;
    BanDb& ban_db_;
    PeersDb peers_db_;
    Acceptor acceptor_;
    Connector connector_;
    
    ThreadInterrupt& interrupt_;
    std::thread thread_dns_seeds_;
    std::thread thread_acceptor_loop_;
    std::thread thread_connector_loop_;
    std::thread thread_message_handler_;
};

#endif // BTCLITE_P2P_H
