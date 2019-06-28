#ifndef BTCLITE_CHAIN_H
#define BTCLITE_CHAIN_H


#include "block.h"
#include "util.h"


/*
 * The block chain is a tree shaped structure starting with the
 * genesis block at the root, with each block potentially having multiple
 * candidates to be the next block. A blockindex may have multiple pprev pointing
 * to it, but at most one of them can be part of the currently active branch.
 */
class BlockIndex {
public:
    
private:
    BlockHeader header_;
    
    // pointer to the index of the predecessor of this block
    BlockIndex *prev_;
};

// An in-memory indexed chain of blocks.
class BlockChain : Uncopyable {
public:
    uint32_t Height() const
    {
        return chain_.size() - 1;
    }
private:
    std::vector<BlockIndex*> chain_;
};

class SingletonBlockChain {
public:
    static BlockChain& GetInstance()
    {
        static BlockChain chain;
        return chain;
    }
    
    SingletonBlockChain(const SingletonBlockChain&) = delete;
    SingletonBlockChain& operator=(const SingletonBlockChain&) = delete;
    
private:
    SingletonBlockChain() {}
};

#endif // BTCLITE_CHAIN_H
