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
        : network_params_(env), network_args_(), ban_db_() {}
    
    bool Init();
    bool Start();
    bool Interrupt();
    bool Stop();
    
    //-------------------------------------------------------------------------
    const Network::Params& network_params() const
    {
        return network_params_;
    }
    
    const NetArgs& network_args() const
    {
        return network_args_;
    }
    
private:
    Network::Params network_params_;
    NetArgs network_args_;
    LocalNetConfig local_network_config_;
    BanDb ban_db_;
    //std::vector<OutboundSession> outbound_sessions_;
    //std::vector<InboundSession> inbound_sessions_;
    Acceptor acceptor_;
    
    ThreadInterrupt interrupt_;
    std::thread thread_dns_seeds_;
    std::thread thread_socket_handler_;
    std::thread thread_open_connections_;
    std::thread thread_message_handler_;
    
    void ThreadDnsSeeds();
    void ThreadOpenConnections(const std::vector<std::string> connect);
    void ThreadSocketHandler();
    void ThreadMessageHandler();
};

#endif // BTCLITE_P2P_H
