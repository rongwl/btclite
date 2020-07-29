#ifndef BTCLITE_CHAIN_CHAIN_STATE_H
#define BTCLITE_CHAIN_CHAIN_STATE_H


#include "block_index.h"
#include "chain/include/params.h"
#include "util.h"


namespace btclite {
namespace chain {

// An in-memory indexed chain of blocks.
class Chain : util::Uncopyable {
public:
    // Returns the index entry for the genesis block of this chain, 
    // or nullptr if none.
    BlockIndex *Genesis() const 
    {
        return active_chain_.size() > 0 ? active_chain_[0] : nullptr;
    }

    // Returns the index entry for the tip of this chain, or nullptr if none.
    BlockIndex *Tip() const 
    {
        return active_chain_.size() > 0 ? active_chain_[active_chain_.size() - 1] : nullptr;
    }
    
    // Returns the index entry at a particular height in this chain,
    // or nullptr if no such height exists.
    BlockIndex *GetBlockIndex(size_t height) const 
    {
        if (height < 0 || height >= active_chain_.size())
            return nullptr;
        return active_chain_[height];
    }

    // Compare two chains efficiently.
    friend bool operator==(const Chain& a, const Chain& b) 
    {
        return a.active_chain_.size() == b.active_chain_.size() &&
               a.active_chain_[a.active_chain_.size() - 1] == b.active_chain_[b.active_chain_.size() - 1];
    }
    
    friend bool operator!=(const Chain& a, const Chain& b)
    {
        return !(a == b);
    }

    // Efficiently check whether a block is present in this chain.
    bool IsExist(const BlockIndex *pindex) const 
    {
        return GetBlockIndex(pindex->height()) == pindex;
    }

    // Find the successor of a block in this chain, 
    // or nullptr if the given index is not found or is the tip.
    BlockIndex *Next(const BlockIndex *pindex) const 
    {
        return GetBlockIndex(pindex->height() + 1);
    }
    
    uint32_t Height() const
    {
        return active_chain_.size() - 1;
    }
    
    // Set/initialize a chain with a given tip.
    void SetTip(const BlockIndex *pindex);

    // Get a BlockLocator that refers to a block in this chain (by default the tip).
    bool GetLocator(consensus::BlockLocator *out, 
                    const BlockIndex *pindex = nullptr) const;

    // Find the last common block between this chain and a block index entry.
    const BlockIndex *FindFork(const BlockIndex *pindex) const;
    
    //-------------------------------------------------------------------------
    const std::vector<BlockIndex*>& active_chain() const
    {
        return active_chain_;
    }
    
private:
    std::vector<BlockIndex*> active_chain_;
};

class ChainState {
public:
    using BlockMap = std::unordered_map<
                     util::Hash256, BlockIndex*, crypto::Hasher<util::Hash256> >;
    
    bool LoadGenesisBlock(const consensus::Block& genesis_block);
    void CheckBlockIndex(const BlockIndex *genesis, const BlockIndex *tip);
    
    uint32_t ActiveChainHeight() const
    {
        LOCK(cs_chain_state_);
        return active_chain_.Height();
    }   
    
private:
    mutable util::CriticalSection cs_chain_state_;
    Chain active_chain_;    
    BlockMap map_block_index_;    
    std::set<BlockIndex*> set_dirty_block_index_;
    std::set<BlockIndex*, BlockIndexWorkComparator> set_block_index_candidates_;    
    BlockIndex *pindex_best_header_ = nullptr;
    
    BlockIndex *AddToBlockIndex(const consensus::BlockHeader& header);
    bool ReceivedBlockTransactions(const consensus::Block& block, BlockIndex *pindex);
    bool ConnectTip(BlockIndex *pindex);
};

} // namesapce chain
} // namesapce btclite

#endif // BTCLITE_CHAIN_CHAIN_STATE_H
