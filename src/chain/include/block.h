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
	BlockHeader(int32_t version, const Hash256& prev_block_hash, const Hash256& merkle_root_hash,
			    uint32_t time, uint32_t nBits, uint32_t nonce)
		: version_(version), 
		  prev_block_hash_(prev_block_hash), merkle_root_hash_(merkle_root_hash),
		  time_(time), nBits_(nBits), nonce_(nonce)
	{
		Hash();
	}
	BlockHeader(const BlockHeader& h)
		: version_(h.version_), 
		  prev_block_hash_(h.prev_block_hash_), merkle_root_hash_(h.merkle_root_hash_),
		  time_(h.time_), nBits_(h.nBits_), nonce_(h.nonce_)
	{
		Hash();
	}
	BlockHeader(BlockHeader&& h)
		: version_(h.version_),
		  prev_block_hash_(std::move(h.prev_block_hash_)),
		  merkle_root_hash_(std::move(h.merkle_root_hash_)),
		  time_(h.time_), nBits_(h.nBits_), nonce_(h.nonce_)
	{
		Hash();
	}
	
	//-------------------------------------------------------------------------
	void SetNull()
	{
		version_ = 0;
		prev_block_hash_.SetNull();
		merkle_root_hash_.SetNull();
		time_ = 0;
		nBits_ = 0;
		nonce_ = 0;
		hash_.SetNull();
	}	
	bool IsNull() const
    {
        return (nBits_ == 0);
    }
	
	//-------------------------------------------------------------------------
	template <typename SType> void Serialize(SType&) const;
	template <typename SType> void UnSerialize(SType&);
	
	//-------------------------------------------------------------------------
	const Hash256& Hash();
	const Hash256& HashCache() const
	{
		return hash_;
	}
	
	//-------------------------------------------------------------------------
	int32_t version() const
	{
		return version_;
	}
	void set_version(int32_t v)
	{
		version_ = v;
	}
	
	const Hash256& hashPrevBlock() const
	{
		return prev_block_hash_;
	}
	void set_hashPrevBlock(const Hash256& hash)
	{
		prev_block_hash_ = hash;
	}
	void set_hashPrevBlock(Hash256&& hash)
	{
		prev_block_hash_ = std::move(hash);
	}
	
	const Hash256& hashMerkleRoot() const
	{
		return merkle_root_hash_;
	}
	void set_hashMerkleRoot(const Hash256& hash)
	{
		merkle_root_hash_ = hash;
	}
	void set_hashMerkleRoot(Hash256&& hash)
	{
		merkle_root_hash_ = std::move(hash);
	}
	
	uint32_t time() const
	{
		return time_;
	}
	void set_time(uint32_t t)
	{
		time_ = t;
	}
	
	uint32_t bits() const
	{
		return nBits_;
	}
	void set_bits(uint32_t b)
	{
		nBits_ = b;
	}
	
	uint32_t nonce() const
	{
		return nonce_;
	}
	void set_nonce(uint32_t n)
	{
		nonce_ = n;
	}
	
private:
	int32_t version_;
	Hash256 prev_block_hash_;
	Hash256 merkle_root_hash_;
	uint32_t time_;
	uint32_t nBits_;
	uint32_t nonce_;
	
	Hash256 hash_;
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
	serial.SerialRead(&version_);
	serial.SerialRead(&prev_block_hash_);
	serial.SerialRead(&merkle_root_hash_);
	serial.SerialRead(&time_);
	serial.SerialRead(&nBits_);
	serial.SerialRead(&nonce_);
}

class Block {
public:
	Block()
	{
		SetNull();
	}
	Block(const BlockHeader& header, const std::vector<Transaction>& transactions)
		: header_(header), transactions_(transactions) {}
	Block(BlockHeader&& header, std::vector<Transaction>&& transactions)
		: header_(std::move(header)), transactions_(std::move(transactions)) {}
	Block(const Block& b)
		: header_(b.header_), transactions_(b.transactions_) {}
	Block(Block&& b)
		: header_(std::move(b.header_)), transactions_(std::move(b.transactions_)) {}
	
	//-------------------------------------------------------------------------
	void SetNull()
	{
		header_.SetNull();
		transactions_.clear();
	}
	std::string ToString() const;
	
	//-------------------------------------------------------------------------
	template <typename SType>
	void Serialize(SType& os) const
	{
		Serializer<SType> serial(os);
		serial.SerialWrite(header_);
		serial.SerialWrite(transactions_);
	}
	template <typename SType>
	void UnSerialize(SType& is)
	{
		Serializer<SType> serial(is);
		serial.SerialRead(&header_);
		serial.SerialRead(&transactions_);
	}

	//-------------------------------------------------------------------------
	const BlockHeader& header()
	{
		return header_;
	}
private:
	BlockHeader header_;
	std::vector<Transaction> transactions_;
};

#endif // BTCLITE_BLOCK_H
