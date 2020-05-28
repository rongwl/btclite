#include "block_index.h"


namespace btclite {
namespace chain {

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

} // namesapce chain
} // namesapce btclite
