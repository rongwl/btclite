#include "block_chain.h"

#include <algorithm>


namespace btclite {
namespace chain {

BlockChain::BlockChain(const util::Configuration& config)
    : params_(config.btcnet()) 
{
}

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

const ChainState& BlockChain::chain_state() const
{
    return chain_state_;
}

ChainState* BlockChain::mutable_chain_state()
{
    return &chain_state_;
}

} // namespace chain
} // namespace btclite
