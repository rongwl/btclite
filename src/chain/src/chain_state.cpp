#include "chain_state.h"


namespace btclite {
namespace chain {

void Chain::SetTip(const BlockIndex *pindex)
{
    if (pindex == nullptr) {
        active_chain_.clear();
        return;
    }
    
    active_chain_.resize(pindex->height() + 1);
    while (pindex && active_chain_[pindex->height()] != pindex) {
        active_chain_[pindex->height()] = const_cast<BlockIndex*>(pindex);
        pindex = pindex->prev();
    }
}

bool Chain::GetLocator(consensus::BlockLocator *out, 
                       const BlockIndex *pindex) const
{
    int step = 1;

    if (!out)
        return false;
    
    out->clear();
    out->reserve(32);

    if (!pindex)
        pindex = Tip();
    while (pindex) {
        out->push_back(pindex->block_hash());
        // Stop when we have added the genesis block.
        if (pindex->height() == 0)
            break;
        // Exponentially larger steps back, plus the genesis block.
        int height = std::max(static_cast<int>(pindex->height()-step), 0);
        if (IsExist(pindex)) {
            // Use O(1) BlockChain index if possible.
            pindex = GetBlockIndex(height);
        } else {
            // Otherwise, use O(log n) skiplist.
            pindex = pindex->GetAncestor(height);
        }
        if (out->size() > 10)
            step *= 2;
    }
    
    return true;
}

const BlockIndex *Chain::FindFork(const BlockIndex *pindex) const
{
    if (pindex == nullptr) {
        return nullptr;
    }
    
    if (pindex->height() > Height())
        pindex = pindex->GetAncestor(Height());
    
    while (pindex && !IsExist(pindex))
        pindex = pindex->prev();
    
    return pindex;
}

BlockIndex *Chain::FindEarliestAtLeast(int64_t time) const
{
    std::vector<BlockIndex*>::const_iterator lower =
        std::lower_bound(active_chain_.begin(), active_chain_.end(), time,
                         [](BlockIndex* pblock, const int64_t& time2) -> 
                         bool { return pblock->time_max() < time2; });
    
    return (lower == active_chain_.end() ? nullptr : *lower);
}

} // namesapce chain
} // namesapce btclite
