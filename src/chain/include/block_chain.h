#ifndef BTCLITE_CHAIN_H
#define BTCLITE_CHAIN_H


#include "block.h"
#include "util.h"


namespace btclite {
namespace chain {

enum BlockStatus: uint32_t {
    //! Unused.
    kBlockValidUnknown      =    0,

    //! Parsed, version ok, hash satisfies claimed PoW, 1 <= vtx count <= max, timestamp not in future
    kBlockValidHeader       =    1,

    //! All parent headers found, difficulty matches, timestamp >= median previous, checkpoint. Implies all parents
    //! are also at least TREE.
    kBlockValidTree         =    2,

    /**
     * Only first tx is coinbase, 2 <= coinbase input script length <= 100, transactions valid, no duplicate txids,
     * sigops, size, merkle root. Implies all parents are at least TREE but not necessarily TRANSACTIONS. When all
     * parent blocks also have TRANSACTIONS, CBlockIndex::nChainTx will be set.
     */
    kBlockValidTransactions =    3,

    //! Outputs do not overspend inputs, no double spends, coinbase output ok, no immature coinbase spends, BIP30.
    //! Implies all parents are also at least CHAIN.
    kBlockValidChain        =    4,

    //! Scripts & signatures ok. Implies all parents are also at least SCRIPTS.
    kBlockValidScripts      =    5,

    //! All validity bits.
    kBlockValidMask         =   kBlockValidHeader | kBlockValidTree | kBlockValidTransactions |
                                 kBlockValidChain | kBlockValidScripts,

    kBlockHaveData          =    8, //!< full block available in blk*.dat
    kBlockHaveUndo          =   16, //!< undo data available in rev*.dat
    kBlockHaveMask          =   kBlockHaveData | kBlockHaveUndo,

    kBlockFailedValid       =   32, //!< stage after last reached validness failed
    kBlockFailedChild       =   64, //!< descends from failed block
    kBlockFailedMask        =   kBlockFailedValid | kBlockFailedChild,

    kBlockOptWitness       =   128, //!< block data in blk*.data was received with a witness-enforcing client
};

/*
 * The block chain is a tree shaped structure starting with the
 * genesis block at the root, with each block potentially having multiple
 * candidates to be the next block. A blockindex may have multiple pprev pointing
 * to it, but at most one of them can be part of the currently active branch.
 */
class BlockIndex {
public:
    BlockIndex() = default;
    
    explicit BlockIndex(const BlockHeader& header)
        : header_(header) {}
    
    //-------------------------------------------------------------------------
    // Check whether this block index entry is valid up to the passed validity level.
    bool IsValid(BlockStatus upto = kBlockValidTransactions) const;
    std::string ToString() const;
    const BlockIndex *GetAncestor(size_t height) const;
    
    //-------------------------------------------------------------------------
    const BlockHeader& header() const
    {
        return header_;
    }
    
    const util::Hash256& block_hash() const
    {
        return block_hash_;
    }
    
    void set_block_hash(const util::Hash256& hash)
    {
        block_hash_ = hash;
    }
    
    const BlockIndex *prev() const
    {
        return prev_;
    }
    
    void set_prev(BlockIndex *prev)
    {
        prev_ = prev;
    }
    
    const BlockIndex* pskip() const
    {
        return pskip_;
    }
    
    size_t height() const
    {
        return height_;
    }
    
    void set_height(size_t height)
    {
        height_ = height;
    }
    
    const util::uint256_t& chain_work() const
    {
        return chain_work_;
    }
    
    uint32_t tx_num() const
    {
        return tx_num_;
    }
    
    uint32_t chain_tx_num() const
    {
        return chain_tx_num_;
    }
    
    BlockStatus status() const
    {
        return status_;
    }
    
    int32_t sequence_id() const
    {
        return sequence_id_;
    }
    
    uint32_t time_max() const
    {
        return time_max_;
    }
    
private:
    BlockHeader header_;
    
    // the hash of this block
    util::Hash256 block_hash_;
    
    // pointer to the index of the predecessor of this block
    BlockIndex *prev_ = nullptr;
    
    // pointer to the index of some further predecessor of this block
    BlockIndex *pskip_ = nullptr;

    // height of the entry in the chain. The genesis block has height 0
    size_t height_ = 0;

    // (memory only) Total amount of work (expected number of hashes) in the chain up to and including this block
    util::uint256_t chain_work_ = 0;
    
    // Number of transactions in this block.
    // Note: in a potential headers-first mode, this number cannot be relied upon
    uint32_t tx_num_ = 0;

    // (memory only) Number of transactions in the chain up to and including this block.
    // This value will be non-zero only if and only if transactions for this block and all its parents are available.
    // Change to 64-bit type when necessary; won't happen before 2030
    uint32_t chain_tx_num_ = 0;

    // Verification status of this block. See enum BlockStatus
    BlockStatus status_ = kBlockValidUnknown;
    
    //! (memory only) Sequential id assigned to distinguish order in which blocks are received.
    int32_t sequence_id_ = 0;

    // (memory only) Maximum nTime in the chain up to and including this block.
    uint32_t time_max_ = 0;
};

class BlockMap {
public:
private:
    mutable util::CriticalSection cs_;
    std::unordered_map<util::Hash256, BlockIndex*, crypto::Hasher<util::Hash256> > map_;
};

// An in-memory indexed chain of blocks.
class BlockChain : util::Uncopyable {
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
    friend bool operator==(const BlockChain& a, const BlockChain& b) 
    {
        return a.active_chain_.size() == b.active_chain_.size() &&
               a.active_chain_[a.active_chain_.size() - 1] == b.active_chain_[b.active_chain_.size() - 1];
    }
    
    friend bool operator!=(const BlockChain& a, const BlockChain& b)
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
    bool GetLocator(BlockLocator *out, const BlockIndex *pindex = nullptr) const;

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
    BlockMap map_block_index_;
};

class SingletonBlockChain : util::Uncopyable {
public:
    static chain::BlockChain& GetInstance()
    {
        static chain::BlockChain chain;
        return chain;
    }
    
private:
    SingletonBlockChain() {}
};

} // namespace chain
} // namespace btclite

#endif // BTCLITE_CHAIN_H
