#ifndef BTCLITE_FULLNODE_EXECUTOR_H
#define BTCLITE_FULLNODE_EXECUTOR_H


#include "chain.h"
#include "chain/include/params.h"
#include "fullnode/include/config.h"
#include "p2p.h"

class FullNodeMain : public Executor {
public:
    FullNodeMain(const FullNodeConfig& config)
        : config_(config), chain_params_(), network_(config) {}

    //-------------------------------------------------------------------------
    bool Init();
    bool Start();
    bool Run();
    void Interrupt();
    void Stop();
    
private:
    const FullNodeConfig& config_;
    Chain::Params chain_params_;
    P2P network_;
    BlockChain block_chain_;

    bool InitNetwork();
};


#endif // BTCLITE_FULLNODE_EXECUTOR_H
