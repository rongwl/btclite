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
        pindex = pindex->pprev();
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
        pindex = pindex->pprev();
    
    return pindex;
}

bool ChainState::LoadGenesisBlock(const consensus::Block& genesis_block)
{
    LOCK(cs_chain_state_);
    
    if (map_block_index_.count(genesis_block.GetHash())) {
        return true;
    }
    
    BlockIndex *pindex = AddToBlockIndex(genesis_block.header());
    if (!ReceivedBlockTransactions(genesis_block, pindex)) {
        BTCLOG(LOG_LEVEL_ERROR) << "Genesis block can not be accepted!";
        return false;
    }
    
    if (!ConnectTip(pindex)) {
        BTCLOG(LOG_LEVEL_ERROR) << "Add genesis block to active chain failed.";
        return false;
    }
    
    return true;
}

BlockIndex* ChainState::AddToBlockIndex(const consensus::BlockHeader& header)
{
    // Check for duplicate
    util::Hash256 hash = header.GetHash();    
    BlockMap::iterator it = map_block_index_.find(hash);
    if (it != map_block_index_.end()) {
        return it->second;
    }
    
    BlockIndex* pindex_new = new BlockIndex(header);
    
    // We assign the sequence id to blocks only when the full data is available,
    // to avoid miners withholding blocks but broadcasting headers, to get a
    // competitive advantage.
    pindex_new->set_sequence_id(0);
    BlockMap::iterator mi = map_block_index_.insert(std::make_pair(hash, pindex_new)).first;
    pindex_new->set_block_hash((*mi).first);
    BlockMap::iterator mi_prev = map_block_index_.find(header.hashPrevBlock());
    if (mi_prev != map_block_index_.end()) {
        pindex_new->set_pprev((*mi_prev).second);
        pindex_new->set_height(pindex_new->pprev()->height() + 1);
        pindex_new->BuildSkip();
    }
    pindex_new->set_chain_work((pindex_new->pprev() ? pindex_new->pprev()->chain_work() : 0) +
                                header.GetBlockProof());
    pindex_new->RaiseValidity(kBlockValidTree);
    
    if (pindex_best_header_ == nullptr || 
            pindex_best_header_->chain_work() < pindex_new->chain_work()) {
        pindex_best_header_ = pindex_new;
    }
    
    set_dirty_block_index_.insert(pindex_new);
    
    return pindex_new;
}

bool ChainState::ReceivedBlockTransactions(const consensus::Block& block, 
                                           BlockIndex *pindex)
{
    // Blocks loaded from disk are assigned id 0, so start the counter at 1.
    static int32_t block_sequence_id = 1;
    /*
     * Every received block is assigned a unique and increasing identifier, so we
     * know which one to give priority in case of a fork.
     */
    util::CriticalSection cs_block_sequence_id;
    
    pindex->set_tx_num(block.transactions().size());
    pindex->set_chain_tx_num(0);
    pindex->set_status(pindex->status() | kBlockHaveData);
    pindex->RaiseValidity(kBlockValidTransactions);
    set_dirty_block_index_.insert(pindex);
    
    if (pindex->pprev() == nullptr || pindex->pprev()->chain_tx_num()) {
        // If pindexNew is the genesis block or all parents are kBlockValidTransactions.
        std::deque<BlockIndex*> queue;
        queue.push_back(pindex);
    
        // Recursively process any descendant blocks that now may be eligible to be connected.
        while (!queue.empty()) {
            BlockIndex *pindex_front = queue.front();
            queue.pop_front();
            pindex_front->set_chain_tx_num((pindex_front->pprev() ? 
                                            pindex_front->pprev()->chain_tx_num() : 0) + 
                                            pindex_front->tx_num());
            {
                LOCK(cs_block_sequence_id);
                pindex->set_sequence_id(block_sequence_id);
                block_sequence_id++;
            }
            if (active_chain_.Tip() == nullptr || 
                    !set_block_index_candidates_.value_comp()(pindex_front, active_chain_.Tip())) {
                set_block_index_candidates_.insert(pindex_front);
            }
        }
    }
    else {
    }
    
    return true;
}

bool ChainState::ConnectTip(BlockIndex *pindex)
{
    assert(pindex->pprev() == active_chain_.Tip());
    
    active_chain_.SetTip(pindex);
    BTCLOG(LOG_LEVEL_VERBOSE) << "active chain new " << pindex->ToString(); 
    
    return true;
}


} // namesapce chain
} // namesapce btclite
