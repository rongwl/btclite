#ifndef BTCLITE_CHAIN_H
#define BTCLITE_CHAIN_H


#include "block.h"


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
private:
    std::vector<BlockIndex*> chain_;
};

#endif // BTCLITE_CHAIN_H
