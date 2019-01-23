#ifndef BTCLITE_BLOCK_H
#define BTCLITE_BLOCK_H

#include "transaction.h"

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
		version_ = 0;
		prev_block_hash_.SetNull();
		merkle_root_hash_.SetNull();
		time_ = 0;
		nBits_ = 0;
		nonce_ = 0;
	}
	
	bool IsNull() const
    {
        return (nBits_ == 0);
    }
	
	template <typename SType>
	void Serialize(SType&) const;
	template <typename SType>
	void UnSerialize(SType&);
	void GetHash(Hash256*) const;
	
private:
	int32_t version_;
	Hash256 prev_block_hash_;
	Hash256 merkle_root_hash_;
	uint32_t time_;
	uint32_t nBits_;
	uint32_t nonce_;
};

template <typename SType>
void BlockHeader::Serialize(SType& os) const
{
	Serializer<SType> serial(os);
	serial.SerialWrite(static_cast<uint32_t>(version_));
	serial.SerialWrite(prev_block_hash_);
	serial.SerialWrite(merkle_root_hash_);
	serial.SerialWrite(time_);
	serial.SerialWrite(nBits_);
	serial.SerialWrite(nonce_);
}

template <typename SType>
void BlockHeader::UnSerialize(SType& is)
{
	Serializer<SType> serial(is);
	serial.SerialRead(version_);
	serial.SerialRead(prev_block_hash_);
	serial.SerialRead(merkle_root_hash_);
	serial.SerialRead(time_);
	serial.SerialRead(nBits_);
	serial.SerialRead(nonce_);
}

#endif // BTCLITE_BLOCK_H
