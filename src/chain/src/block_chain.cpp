#include "block_chain.h"

#include <algorithm>


namespace btclite {
namespace chain {

// Turn the lowest '1' bit in the binary representation of a number into a '0'.
inline int InvertLowestOne(int n)
{
    return n & (n - 1); 
}

// Compute what height to jump back to with the CBlockIndex::pskip pointer.
inline int GetSkipHeight(size_t height) {
    if (height < 2)
        return 0;

    // Determine which height to jump back to. Any number strictly lower than height is acceptable,
    // but the following expression seems to perform well in simulations (max 110 steps to go back
    // up to 2**18 blocks).
    return (height & 1) ? 
           InvertLowestOne(InvertLowestOne(height - 1)) + 1 : InvertLowestOne(height);
}

bool BlockIndex::IsValid(BlockStatus upto) const
{
    assert(!(upto & kBlockValidMask)); // Only validity flags allowed.
    if (status_ & kBlockFailedMask)
        return false;
    return ((status_ & kBlockValidMask) >= upto);
}

std::string BlockIndex::ToString() const
{
    std::stringstream ss;
    ss << "BlockIndex(pprev=" << prev_ << ", height=" << height_
       << ", merkle=" << header_.hashMerkleRoot().ToString()
       << ", hashBlock=" << block_hash_.ToString() << ")";
    
    return ss.str();
}

const BlockIndex *BlockIndex::GetAncestor(size_t height) const
{
    if (height > height_ || height < 0) {
        return nullptr;
    }

    const BlockIndex* pindex_walk = this;
    size_t height_walk = height_;
    while (height_walk > height) {
        int height_skip = GetSkipHeight(height_walk);
        int height_skip_prev = GetSkipHeight(height_walk - 1);
        if (pindex_walk->pskip() != nullptr &&
                (height_skip == height ||
                 (height_skip > height && !(height_skip_prev < height_skip - 2 &&
                                           height_skip_prev >= height)))) {
            // Only follow pskip if pprev->pskip isn't better than pskip->pprev.
            pindex_walk = pindex_walk->pskip();
            height_walk = height_skip;
        } else {
            assert(pindex_walk->prev());
            pindex_walk = pindex_walk->prev();
            height_walk--;
        }
    }
    
    return pindex_walk;
}

void BlockChain::SetTip(const BlockIndex *pindex)
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

bool BlockChain::GetLocator(BlockLocator *out, const BlockIndex *pindex) const
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

const BlockIndex *BlockChain::FindFork(const BlockIndex *pindex) const
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

BlockIndex *BlockChain::FindEarliestAtLeast(int64_t time) const
{
    std::vector<BlockIndex*>::const_iterator lower =
        std::lower_bound(active_chain_.begin(), active_chain_.end(), time,
            [](BlockIndex* pblock, const int64_t& time2) -> 
            bool { return pblock->time_max() < time2; });
    
    return (lower == active_chain_.end() ? nullptr : *lower);
}

} // namespace chain
} // namespace btclite
