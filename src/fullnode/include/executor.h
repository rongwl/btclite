#ifndef BTCLITE_FULLNODE_EXECUTOR_H
#define BTCLITE_FULLNODE_EXECUTOR_H


#include "chain.h"
#include "chain/include/params.h"
#include "fullnode/include/config.h"
#include "p2p.h"


class FullNodeMain : public Executor {
public:
    FullNodeMain(BaseEnv env)
        : chain_params_(env), network_(env) {}

    //-------------------------------------------------------------------------
    bool Init();
    bool Start();
    bool Run();
    void Interrupt();
    void Stop();
    
private:
    Chain::Params chain_params_;
    P2P network_;
    BlockChain block_chain_;

    bool InitNetwork();
};


#endif // BTCLITE_FULLNODE_EXECUTOR_H
