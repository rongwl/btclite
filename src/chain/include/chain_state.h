#ifndef BTCLITE_CHAIN_STATE_H
#define BTCLITE_CHAIN_STATE_H


#include "block_chain.h"


namespace btclite {
namespace chain {

class BlockMap {
public:
private:
    mutable util::CriticalSection cs_;
    std::unordered_map<crypto::Hash256, BlockIndex*, crypto::Hasher<crypto::Hash256> > map_;
};

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
    
    const BlockMap& map_block_index() const
    {
        return map_block_index_;
    }
    
    BlockMap *mutable_map_block_index()
    {
        return &map_block_index_;
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
    BlockMap map_block_index_;
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
