#include "block.h"

#include <memory>
#include <sstream>


namespace btclite {
namespace chain {

const crypto::Hash256& BlockHeader::Hash() const
{
    if (hash_cache_.IsNull()) {
        crypto::HashOStream hs;
        hs << *this;
        hs.DoubleSha256(&hash_cache_);
    }
    
    return hash_cache_;
}

BlockHeader& BlockHeader::operator=(const BlockHeader& b)
{
    version_ = b.version_;
    prev_block_hash_ = b.prev_block_hash_;
    merkle_root_hash_ = b.merkle_root_hash_;
    time_ = b.time_;
    nBits_ = b.nBits_;
    nonce_ = b.nonce_;
    hash_cache_.Clear();
    
    return *this;
}

BlockHeader& BlockHeader::operator=(BlockHeader&& b) noexcept
{
    if (this != &b) {
        version_ = b.version_;
        prev_block_hash_ = std::move(b.prev_block_hash_);
        merkle_root_hash_ = std::move(b.merkle_root_hash_);
        time_ = b.time_;
        nBits_ = b.nBits_;
        nonce_ = b.nonce_;
        hash_cache_.Clear();
    }
    
    return *this;
}

Block& Block::operator=(Block&& b) noexcept
{
    if (this != &b) {
    
    }
    
    return *this;
}

std::string Block::ToString() const
{
    std::stringstream ss;
    ss << "Block(hash=" << header_.Hash().ToString().substr(0,10) << ", "
       << "ver=" << "0x" << std::hex << std::setw(8) << std::setfill('0') << header_.version() << ", "
       << "hashPrevBlock=" << header_.hashPrevBlock().ToString().substr(0,10) << ", "
       << "hashMerkleRoot=" << header_.hashMerkleRoot().ToString().substr(0,10) << ", "
       << "nTime=" << std::dec << header_.time() << ", "
       << "bits=" << std::hex << std::setw(8) << std::setfill('0') << header_.bits() << ", "
       << "nNonce=" << std::dec << header_.nonce() << ", "
       << "tx.size=" << transactions_.size() << ")\n";

    for (const auto& tx : transactions_) {
        ss << "  " << tx.ToString() << "\n";
    }
    
    return ss.str();
}

crypto::Hash256 Block::ComputeMerkleRoot() const
{
    std::vector<crypto::Hash256> leaves;
    std::vector<crypto::Hash256> swap;
    for_each(transactions_.begin(), transactions_.end(), [&leaves](const Transaction& tx)
                                                         { leaves.push_back(tx.Hash()); });
    
    swap.reserve((leaves.size() + 1) / 2);
    while (leaves.size() > 1) {
        // If number of hashes is odd, duplicate last hash in the list.
        if (leaves.size() % 2 != 0)
            leaves.push_back(leaves.back());
        for (auto it = leaves.begin(); it != leaves.end(); it += 2) {
            crypto::Hash256 hash;
            crypto::DoubleSha256(it[0].ToString()+it[1].ToString(), &hash);
            swap.push_back(hash);
        }
        std::swap(leaves, swap);
        swap.clear();
    }
    
    // There is now only one item in the list.
    return leaves.front();
}

Block CreateGenesisBlock(const std::string& coinbase, const Script& output_script,
                         uint32_t time, uint32_t nonce, uint32_t bits, int32_t version,
                         uint64_t reward)
{
    Script script;    
    script.Push(0x1d00ffff);
    script.Push(ScriptInt(4));
    script.Push(std::vector<uint8_t>(coinbase.begin(), coinbase.end()));

    std::vector<TxIn> inputs;
    TxIn input(std::move(OutPoint()), std::move(script));
    inputs.push_back(std::move(input));

    std::vector<TxOut> outputs;
    outputs.push_back(TxOut(reward, output_script));

    std::vector<Transaction> transactions;
    Transaction tx_new(1, std::move(inputs), std::move(outputs), 0);
    transactions.push_back(std::move(tx_new));

    Block genesis(std::move(transactions));
    BlockHeader header(version, crypto::Hash256(), genesis.ComputeMerkleRoot(), time, bits, nonce);    
    genesis.set_header(std::move(header));
    
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * Block(hash=0000000000, ver=0x00000001, hashPrevBlock=0000000000, hashMerkleRoot=4a5e1e4baa, nTime=1231006505, bits=1d00ffff, nNonce=2083236893, tx.size=1)
 *   Transaction(hash=4a5e1e4baa, ver=1, inputs.size=1, outputs.size=1, lock_time=0)
 *     TxIn(OutPoint(0000000000, 4294967295), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     TxOut(value=50.00000000, scriptPubKey=4104678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5fac)
 *   vMerkleTree: 4a5e1e
 */
Block CreateGenesisBlock(uint32_t time, uint32_t nonce, uint32_t bits, int32_t version, uint64_t reward)
{
    const std::string coinbase = "The Times 03/Jan/2009 Chancellor on brink of second bailout for banks";
    Script output_script;
    output_script.Push(
        util::DecodeHex(
            "04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f"));
    output_script.Push(Opcode::OP_CHECKSIG);

    return CreateGenesisBlock(coinbase, output_script, time, nonce, bits, version, reward);
}

} // namespace chain
} // namespace btclite
