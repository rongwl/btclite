#ifndef BTCLITE_FULLNODE_EXECUTOR_H
#define BTCLITE_FULLNODE_EXECUTOR_H


#include "chain/include/params.h"
#include "fullnode/include/config.h"
#include "p2p.h"


class FullNodeMain : public btclite::util::Executor {
public:
    explicit FullNodeMain(const FullNodeConfig& config)
        : network_(config)
    {
        btclite::chain::SingletonParams::GetInstance(config.btcnet());
        //SingletonChainState::GetInstance();
    }

    //-------------------------------------------------------------------------
    bool Init();
    bool Start();
    void Interrupt();
    void Stop();
    
private:
    btclite::network::P2P network_;
};


#endif // BTCLITE_FULLNODE_EXECUTOR_H
