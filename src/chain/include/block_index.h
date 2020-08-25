#ifndef BTCLITE_CHAIN_BLOCK_INDEX_H
#define BTCLITE_CHAIN_BLOCK_INDEX_H


#include "block.h"


namespace btclite {
namespace chain {

enum BlockStatus : uint32_t {
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
    
    explicit BlockIndex(const consensus::BlockHeader& header)
        : header_(header) {}
    
    //-------------------------------------------------------------------------
    // Check whether this block index entry is valid up to the passed validity level.
    bool IsValid(BlockStatus upto = kBlockValidTransactions) const;
    std::string ToString() const;
    const BlockIndex *GetAncestor(size_t height) const;
    //! Raise the validity level of this block index entry.
    //! Returns true if the validity was changed.
    bool RaiseValidity(BlockStatus up_to);
    
    BlockIndex *GetAncestor(size_t height)
    {
        return const_cast<BlockIndex*>(GetAncestor(height));
    }
    
    void BuildSkip()
    {
        if (pprev_) {
            pskip_ = pprev_->GetAncestor(GetSkipHeight(height_));
        }
    }
    
    //-------------------------------------------------------------------------
    const consensus::BlockHeader& header() const
    {
        return header_;
    }
    
    util::Hash256 block_hash() const
    {
        return block_hash_;
    }
    
    void set_block_hash(const util::Hash256& hash)
    {
        block_hash_ = hash;
    }
    
    const BlockIndex *pprev() const
    {
        return pprev_;
    }
    
    void set_pprev(BlockIndex *pprev)
    {
        pprev_ = pprev;
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
    
    void set_chain_work(const util::uint256_t& chain_work)
    {
        chain_work_ = chain_work;
    }
    
    uint32_t tx_num() const
    {
        return tx_num_;
    }
    
    void set_tx_num(uint32_t num)
    {
        tx_num_ = num;
    }
    
    uint32_t chain_tx_num() const
    {
        return chain_tx_num_;
    }
    
    void set_chain_tx_num(uint32_t num)
    {
        chain_tx_num_ = num;
    }
    
    std::underlying_type_t<BlockStatus> status() const
    {
        return status_;
    }
    
    void set_status(std::underlying_type_t<BlockStatus> status)
    {
        status_ = status;
    }
    
    int32_t sequence_id() const
    {
        return sequence_id_;
    }
    
    void set_sequence_id(int32_t id)
    {
        sequence_id_ = id;
    }
    
private:
    consensus::BlockHeader header_;
    
    // the hash of this block
    util::Hash256 block_hash_;
    
    // pointer to the index of the predecessor of this block
    BlockIndex *pprev_ = nullptr;
    
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
    std::underlying_type_t<BlockStatus> status_ = kBlockValidUnknown;
    
    //! (memory only) Sequential id assigned to distinguish order in which blocks are received.
    int32_t sequence_id_ = 0;

    
    // Turn the lowest '1' bit in the binary representation of a number into a '0'.
    inline int InvertLowestOne(int n) const
    {
        return n & (n - 1); 
    }

    // Compute what height to jump back to with the CBlockIndex::pskip pointer.
    inline int GetSkipHeight(size_t height) const
    {
        if (height < 2)
            return 0;

        // Determine which height to jump back to. Any number strictly lower than height is acceptable,
        // but the following expression seems to perform well in simulations (max 110 steps to go back
        // up to 2**18 blocks).
        return (height & 1) ? 
               InvertLowestOne(InvertLowestOne(height - 1)) + 1 : InvertLowestOne(height);
    }
};

struct BlockIndexWorkComparator
{
    bool operator()(const BlockIndex *pa, const BlockIndex *pb) const 
    {
        // First sort by most total work, ...
        if (pa->chain_work() > pb->chain_work()) {
            return false;
        }
        if (pa->chain_work() < pb->chain_work()) {
            return true;
        }

        // ... then by earliest time received, ...
        if (pa->sequence_id() < pb->sequence_id()) {
            return false;
        }
        if (pa->sequence_id() > pb->sequence_id()) {
            return true;
        }

        // Use pointer address as tie breaker (should only happen with blocks
        // loaded from disk, as those all have id 0).
        if (pa < pb) {
            return false;
        }
        if (pa > pb) {
            return true;
        }

        // Identical blocks.
        return false;
    }
};

} // namesapce chain
} // namesapce btclite

#endif // BTCLITE_CHAIN_BLOCK_INDEX_H
