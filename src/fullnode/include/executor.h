#ifndef BTCLITE_FULLNODE_EXECUTOR_H
#define BTCLITE_FULLNODE_EXECUTOR_H


#include "chain/include/params.h"
#include "block_chain.h"
#include "fullnode/include/config.h"
#include "p2p.h"


namespace btclite {
namespace fullnode {

class FullNode final : public btclite::util::Executor {
public:
    FullNode(const FullNodeConfig& config)
        : chain_(config), network_(config) {}

    //-------------------------------------------------------------------------
    bool Init();
    bool Start();
    void Interrupt();
    void Stop();
    
private:
    std::thread::id thread_id_;
    btclite::chain::BlockChain chain_;
    btclite::network::P2P network_;
    
    bool BasicSetupCustomized()
    {
        return true;
    }
};

} // namespace fullnode
} // namespace btclite


#endif // BTCLITE_FULLNODE_EXECUTOR_H
