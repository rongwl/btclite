#ifndef BTCLITE_P2P_H
#define BTCLITE_P2P_H

#include <thread>

#include "acceptor.h"
#include "bandb.h"
#include "connector.h"
#include "network/include/params.h"
#include "thread.h"

class P2P {
public:
    P2P(const ExecutorConfig& config);
    
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
    
    static SipHasher& GetDeterministicRandomizer(uint64_t id)
    {
        return sip_hash_.Update(id);
    }
    
private:
    const ExecutorConfig& executor_config_;
    Network::Params network_params_;
    NetArgs network_args_;
    LocalNetConfig local_network_config_;
    BanDb ban_db_;
    //std::vector<OutboundSession> outbound_sessions_;
    //std::vector<InboundSession> inbound_sessions_;
    Acceptor acceptor_;
    
    // for generating deterministic random number
    static SipHasher sip_hash_;
    
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
