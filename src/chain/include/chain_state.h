#ifndef BTCLITE_CHAIN_CHAIN_STATE_H
#define BTCLITE_CHAIN_CHAIN_STATE_H


#include "block_index.h"
#include "util.h"


namespace btclite {
namespace chain {

class BlockMap {
public:
private:
    mutable util::CriticalSection cs_;
    std::unordered_map<util::Hash256, BlockIndex*, crypto::Hasher<util::Hash256> > map_;
};

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

    // Find the earliest block with timestamp equal or greater than the given.
    BlockIndex *FindEarliestAtLeast(int64_t time) const;
    
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
    const Chain& active_chain() const
    {
        return active_chain_;
    }
    
    Chain *mutable_active_chain()
    {
        return &active_chain_;
    }
    
    const BlockIndex *pindex_best_header() const
    {
        LOCK(cs_);
        return pindex_best_header_;
    }
    
    void set_pindex_best_header(BlockIndex *index)
    {
        LOCK(cs_);
        pindex_best_header_ = index;
    }
    
private:
    Chain active_chain_;
    BlockMap map_block_index_;
    
    mutable util::CriticalSection cs_;
    BlockIndex *pindex_best_header_ = nullptr;
};

} // namesapce chain
} // namesapce btclite

#endif // BTCLITE_CHAIN_CHAIN_STATE_H
