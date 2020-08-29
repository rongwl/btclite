#ifndef BTCLITE_CONSENSUS_BLOCK_H
#define BTCLITE_CONSENSUS_BLOCK_H


#include "transaction.h"


namespace btclite {
namespace consensus {

using BlockLocator = std::vector<util::Hash256>;

/** Nodes collect new transactions into a block, hash them into a hash tree,
 * and scan through nonce values to make the block's hash satisfy proof-of-work
 * requirements.  When they solve the proof-of-work, they broadcast the block
 * to everyone and the block is added to the block chain.  The first transaction
 * in the block is a special one that creates a new coin owned by the creator
 * of the block.
 */
class BlockHeader {
public:
    BlockHeader();    
    BlockHeader(int32_t version, const util::Hash256& prev_block_hash, 
                const util::Hash256& merkle_root_hash,
                uint32_t time, uint32_t bits, uint32_t nonce);    
    BlockHeader(const BlockHeader& h);
    
    //-------------------------------------------------------------------------
    void Clear();    
    bool IsNull() const;    
    util::uint256_t GetBlockProof() const;
    
    //-------------------------------------------------------------------------
    template <typename Stream> void Serialize(Stream& os) const;
    template <typename Stream> void Deserialize(Stream& is);
    
    //-------------------------------------------------------------------------
    util::Hash256 GetHash() const;
    
    //-------------------------------------------------------------------------
    BlockHeader& operator=(const BlockHeader& b);
    BlockHeader& operator=(BlockHeader&& b) noexcept;
    
    //-------------------------------------------------------------------------
    int32_t version() const;
    void set_version(int32_t v);
    
    util::Hash256 hashPrevBlock() const;
    void set_hashPrevBlock(const util::Hash256& hash);
    
    util::Hash256 hashMerkleRoot() const;
    void set_hashMerkleRoot(const util::Hash256& hash);
    
    uint32_t time() const;
    void set_time(uint32_t t);
    
    uint32_t bits() const;
    void set_bits(uint32_t b);
    
    uint32_t nonce() const;
    void set_nonce(uint32_t n);
    
private:
    int32_t version_;
    util::Hash256 prev_block_hash_;
    util::Hash256 merkle_root_hash_;
    uint32_t time_;
    uint32_t nBits_;
    uint32_t nonce_;
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
    Block();
    
    Block(const std::vector<Transaction>& transactions);
    Block(std::vector<Transaction>&& transactions) noexcept;
    
    Block(const BlockHeader& header, const std::vector<Transaction>& transactions);
    Block(BlockHeader&& header, std::vector<Transaction>&& transactions) noexcept;
    
    Block(const Block& b);
    Block(Block&& b) noexcept;
    
    //-------------------------------------------------------------------------
    void Clear();    
    util::Hash256 GetHash() const;    
    std::string ToString() const;
    util::Hash256 ComputeMerkleRoot() const;
    
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
    const BlockHeader& header() const;
    void set_header(const BlockHeader& header);
    void set_header(BlockHeader&& header);
    
    const std::vector<Transaction>& transactions() const;
    void set_transactions(const std::vector<Transaction>& transactions);
    void set_transactions(std::vector<Transaction>&& transactions);
    
private:
    BlockHeader header_;
    std::vector<Transaction> transactions_;
};


Block CreateGenesisBlock(const std::string& coinbase, const Script& output_script, uint32_t time,
                         uint32_t nonce, uint32_t bits, int32_t version, uint64_t reward);
Block CreateGenesisBlock(uint32_t time, uint32_t nonce, uint32_t bits, int32_t version, uint64_t reward);

} // namespace consensus
} // namespace btclite

#endif // BTCLITE_CONSENSUS_BLOCK_H
