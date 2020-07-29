#include "block_chain.h"

#include <algorithm>


namespace btclite {
namespace chain {

bool BlockChain::Init()
{
    BTCLOG(LOG_LEVEL_INFO) << "Initializing block chain...";
    
    chain_state_.LoadGenesisBlock(params_.consensus_params().GenesisBlock());
    
    BTCLOG(LOG_LEVEL_INFO) << "Finished initializing block chain.";
    
    return true;
}

bool BlockChain::Start()
{
    return true;
}

void BlockChain::Interrupt()
{
}

void BlockChain::Stop()
{
}

} // namespace chain
} // namespace btclite
