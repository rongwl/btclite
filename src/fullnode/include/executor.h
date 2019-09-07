#ifndef BTCLITE_FULLNODE_EXECUTOR_H
#define BTCLITE_FULLNODE_EXECUTOR_H


#include "chain.h"
#include "chain/include/params.h"
#include "fullnode/include/config.h"
#include "p2p.h"


class FullNodeMain : public Executor {
public:
    FullNodeMain(const FullNodeConfig& config)
        : chain_params_(Chain::SingletonParams::GetInstance(config.env())),
          network_(config), block_chain_(SingletonBlockChain::GetInstance()) {}

    //-------------------------------------------------------------------------
    bool Init();
    bool Start();
    void Interrupt();
    void Stop();
    
private:
    Chain::Params& chain_params_;
    P2P network_;
    BlockChain& block_chain_;
};


#endif // BTCLITE_FULLNODE_EXECUTOR_H
