#ifndef BTCLITE_BLOCK_H
#define BTCLITE_BLOCK_H

#include "transaction.h"


namespace btclite {
namespace chain {

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
        Clear();
    }
    
    BlockHeader(int32_t version, const crypto::Hash256& prev_block_hash, const crypto::Hash256& merkle_root_hash,
                uint32_t time, uint32_t bits, uint32_t nonce)
        : version_(version), 
          prev_block_hash_(prev_block_hash), merkle_root_hash_(merkle_root_hash),
          time_(time), nBits_(bits), nonce_(nonce)
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
    
    //-------------------------------------------------------------------------
    void Clear()
    {
        version_ = 0;
        prev_block_hash_.Clear();
        merkle_root_hash_.Clear();
        time_ = 0;
        nBits_ = 0;
        nonce_ = 0;
        hash_cache_.Clear();
    }    
    bool IsNull() const
    {
        return (nBits_ == 0);
    }
    
    //-------------------------------------------------------------------------
    template <typename Stream> void Serialize(Stream& os) const;
    template <typename Stream> void Deserialize(Stream& is);
    
    //-------------------------------------------------------------------------
    const crypto::Hash256& Hash() const;
    
    //-------------------------------------------------------------------------
    BlockHeader& operator=(const BlockHeader& b);
    BlockHeader& operator=(BlockHeader&& b) noexcept;
    
    //-------------------------------------------------------------------------
    int32_t version() const
    {
        return version_;
    }
    void set_version(int32_t v)
    {
        version_ = v;
        hash_cache_.Clear();
    }
    
    const crypto::Hash256& hashPrevBlock() const
    {
        return prev_block_hash_;
    }
    void set_hashPrevBlock(const crypto::Hash256& hash)
    {
        prev_block_hash_ = hash;
        hash_cache_.Clear();
    }
    
    const crypto::Hash256& hashMerkleRoot() const
    {
        return merkle_root_hash_;
        hash_cache_.Clear();
    }
    void set_hashMerkleRoot(const crypto::Hash256& hash)
    {
        merkle_root_hash_ = hash;
        hash_cache_.Clear();
    }
    
    uint32_t time() const
    {
        return time_;
    }
    void set_time(uint32_t t)
    {
        time_ = t;
        hash_cache_.Clear();
    }
    
    uint32_t bits() const
    {
        return nBits_;
    }
    void set_bits(uint32_t b)
    {
        nBits_ = b;
        hash_cache_.Clear();
    }
    
    uint32_t nonce() const
    {
        return nonce_;
    }
    void set_nonce(uint32_t n)
    {
        nonce_ = n;
        hash_cache_.Clear();
    }
    
private:
    int32_t version_;
    crypto::Hash256 prev_block_hash_;
    crypto::Hash256 merkle_root_hash_;
    uint32_t time_;
    uint32_t nBits_;
    uint32_t nonce_;
    
    mutable crypto::Hash256 hash_cache_;
};

template <typename Stream>
void BlockHeader::Serialize(Stream& os) const
{
    util::Serializer<Stream> serializer(os);
    serializer.SerialWrite(static_cast<uint32_t>(version_));
    serializer.SerialWrite(prev_block_hash_);
    serializer.SerialWrite(merkle_root_hash_);
    serializer.SerialWrite(time_);
    serializer.SerialWrite(nBits_);
    serializer.SerialWrite(nonce_);
}

template <typename Stream>
void BlockHeader::Deserialize(Stream& is)
{
    util::Deserializer<Stream> deserializer(is);
    deserializer.SerialRead(&version_);
    deserializer.SerialRead(&prev_block_hash_);
    deserializer.SerialRead(&merkle_root_hash_);
    deserializer.SerialRead(&time_);
    deserializer.SerialRead(&nBits_);
    deserializer.SerialRead(&nonce_);
}

class Block {
public:
    Block()
    {
        Clear();
    }
    
    Block(const std::vector<Transaction>& transactions)
        : header_(BlockHeader()), transactions_(transactions) {}
    Block(std::vector<Transaction>&& transactions) noexcept
        : header_(BlockHeader()), transactions_(std::move(transactions)) {}
    
    Block(const BlockHeader& header, const std::vector<Transaction>& transactions)
        : header_(header), transactions_(transactions) {}
    Block(BlockHeader&& header, std::vector<Transaction>&& transactions) noexcept
        : header_(std::move(header)), transactions_(std::move(transactions)) {}
    
    Block(const Block& b)
        : header_(b.header_), transactions_(b.transactions_) {}
    Block(Block&& b) noexcept
        : header_(std::move(b.header_)), transactions_(std::move(b.transactions_)) {}
    
    //-------------------------------------------------------------------------
    void Clear()
    {
        header_.Clear();
        transactions_.clear();
    }
    std::string ToString() const;
    crypto::Hash256 ComputeMerkleRoot() const;
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& os) const
    {
        util::Serializer<Stream> serializer(os);
        serializer.SerialWrite(header_);
        serializer.SerialWrite(transactions_);
    }
    template <typename Stream>
    void Deserialize(Stream& is)
    {
        util::Deserializer<Stream> deserializer(is);
        deserializer.SerialRead(&header_);
        deserializer.SerialRead(&transactions_);
    }
    
    //-------------------------------------------------------------------------
    // This class is move assignable but NOT copy assignable (performance).
    Block& operator=(const Block& b) = delete;
    Block& operator=(Block&& b) noexcept;

    //-------------------------------------------------------------------------
    const BlockHeader& header() const
    {
        return header_;
    }
    void set_header(const BlockHeader& header)
    {
        header_ = header;
    }
    void set_header(BlockHeader&& header)
    {
        header_ = std::move(header);
    }
    
    const std::vector<Transaction>& transactions() const
    {
        return transactions_;
    }
    void set_transactions(const std::vector<Transaction>& transactions)
    {
        transactions_ = transactions;
    }
    void set_transactions(std::vector<Transaction>&& transactions)
    {
        transactions_ = std::move(transactions);
    }
    
private:
    BlockHeader header_;
    std::vector<Transaction> transactions_;
};

Block CreateGenesisBlock(const std::string& coinbase, const Script& output_script, uint32_t time,
                         uint32_t nonce, uint32_t bits, int32_t version, uint64_t reward);
Block CreateGenesisBlock(uint32_t time, uint32_t nonce, uint32_t bits, int32_t version, uint64_t reward);

} // namespace chain
} // namespace btclite

#endif // BTCLITE_BLOCK_H
