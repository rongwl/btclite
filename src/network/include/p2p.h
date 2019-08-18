#ifndef BTCLITE_P2P_H
#define BTCLITE_P2P_H

#include <thread>

#include "acceptor.h"
#include "bandb.h"
#include "connector.h"
#include "network/include/params.h"
#include "thread.h"


class P2P : Uncopyable {
public:
    P2P(BaseEnv env)
        : network_params_(Network::SingletonParams::GetInstance(env)),
          network_args_(), local_network_config_(SingletonLocalNetCfg::GetInstance()), 
          nodes_(SingletonNodes::GetInstance()),
          ban_db_(SingletonBanDb::GetInstance()), acceptor_(),
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
    Acceptor acceptor_;
    
    ThreadInterrupt& interrupt_;
    std::thread thread_dns_seeds_;
    std::thread thread_acceptor_loop_;
    std::thread thread_open_connections_;
    std::thread thread_message_handler_;
    
    void ThreadDnsSeeds();
    void ThreadOpenConnections(const std::vector<std::string> connect);
    void ThreadSocketHandler();
    void ThreadMessageHandler();
};

#endif // BTCLITE_P2P_H
