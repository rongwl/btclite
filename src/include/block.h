#ifndef BTCLITE_BLOCK_H
#define BTCLITE_BLOCK_H

#include "uint256.h"

/** Nodes collect new transactions into a block, hash them into a hash tree,
 * and scan through nonce values to make the block's hash satisfy proof-of-work
 * requirements.  When they solve the proof-of-work, they broadcast the block
 * to everyone and the block is added to the block chain.  The first transaction
 * in the block is a special one that creates a new coin owned by the creator
 * of the block.
 */
class BlockHeader {
public:
	BlockHeader()
	{
		SetNull();
	}
	
	void SetNull()
	{
		version = 0;
		prev_block_hash.SetNull();
		merkle_root_hash.SetNull();
		time = 0;
		nBits = 0;
		nonce = 0;
	}
	
	bool IsNull() const
    {
        return (nBits == 0);
    }
	
	uint256 GetHash() const;
private:
	int32_t version;
	uint256 prev_block_hash;
	uint256 merkle_root_hash;
	uint32_t time;
	uint32_t nBits;
	uint32_t nonce;
};

#endif // BTCLITE_BLOCK_H
