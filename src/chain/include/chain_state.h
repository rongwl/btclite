#ifndef BTCLITE_CHAIN_STATE_H
#define BTCLITE_CHAIN_STATE_H


#include "block_chain.h"


namespace btclite {
namespace chain {



class ChainState : util::Uncopyable {
public:    
    const BlockChain& active_chain() const
    {
        return active_chain_;
    }
    
    BlockChain *mutable_active_chain()
    {
        return &active_chain_;
    }
    
    const BlockIndex *best_header_block_index() const
    {
        LOCK(cs_);
        return best_header_block_index_;
    }
    
    void set_best_header_block_index(BlockIndex *index)
    {
        LOCK(cs_);
        best_header_block_index_ = index;
    }
    
private:
    mutable util::CriticalSection cs_;
    BlockChain active_chain_;
    BlockIndex *best_header_block_index_ = nullptr;
};

class SingletonChainState : util::Uncopyable {
public:
    static ChainState& GetInstance()
    {
        static ChainState chain_state;
        return chain_state;
    }

private:
    SingletonChainState() {}
};

} // namespace chain
} // namespace btclite

#endif // BTCLITE_CHAIN_STATE_H
