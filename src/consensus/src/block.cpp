#include "block.h"

#include <memory>
#include <sstream>

#include "compact.h"


namespace btclite {
namespace consensus {

BlockHeader::BlockHeader()
{
    Clear();
}
      
BlockHeader::BlockHeader(int32_t version, const util::Hash256& prev_block_hash, 
            const util::Hash256& merkle_root_hash,
            uint32_t time, uint32_t bits, uint32_t nonce)
    : version_(version), 
      prev_block_hash_(prev_block_hash), merkle_root_hash_(merkle_root_hash),
      time_(time), nBits_(bits), nonce_(nonce)
{
}
      
BlockHeader::BlockHeader(const BlockHeader& h)
    : version_(h.version_), 
      prev_block_hash_(h.prev_block_hash_), merkle_root_hash_(h.merkle_root_hash_),
      time_(h.time_), nBits_(h.nBits_), nonce_(h.nonce_)
{
}

void BlockHeader::Clear()
{
    version_ = 0;
    prev_block_hash_.fill(0);
    merkle_root_hash_.fill(0);
    time_ = 0;
    nBits_ = 0;
    nonce_ = 0;
}

bool BlockHeader::IsNull() const
{
    return (nBits_ == 0);
}

util::uint256_t BlockHeader::GetBlockProof() const
{
    Compact compact(nBits_);
    util::uint256_t target = compact.normal();
    
    if (compact.negative() || 
            compact.overflowed() || 
            target == 0) {
        return 0;
    }
    
    // We need to compute 2**256 / (target+1), but we can't represent 2**256
    // as it's too large for an util::uint256_t. However, as 2**256 is at least
    // as large as target+1, it is equal to ((2**256 - target - 1) / (target+1)) + 1,
    // or ~target / (target+1) + 1.
    return (~target / (target + 1)) + 1;
}

util::Hash256 BlockHeader::GetHash() const
{
    return crypto::GetDoubleHash(*this);
}

BlockHeader& BlockHeader::operator=(const BlockHeader& b)
{
    version_ = b.version_;
    prev_block_hash_ = b.prev_block_hash_;
    merkle_root_hash_ = b.merkle_root_hash_;
    time_ = b.time_;
    nBits_ = b.nBits_;
    nonce_ = b.nonce_;
    
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
    }
    
    return *this;
}

int32_t BlockHeader::version() const
{
    return version_;
}

void BlockHeader::set_version(int32_t v)
{
    version_ = v;
}

util::Hash256 BlockHeader::hashPrevBlock() const
{
    return prev_block_hash_;
}

void BlockHeader::set_hashPrevBlock(const util::Hash256& hash)
{
    prev_block_hash_ = hash;
}

util::Hash256 BlockHeader::hashMerkleRoot() const
{
    return merkle_root_hash_;
}

void BlockHeader::set_hashMerkleRoot(const util::Hash256& hash)
{
    merkle_root_hash_ = hash;
}

uint32_t BlockHeader::time() const
{
    return time_;
}

void BlockHeader::set_time(uint32_t t)
{
    time_ = t;
}

uint32_t BlockHeader::bits() const
{
    return nBits_;
}

void BlockHeader::set_bits(uint32_t b)
{
    nBits_ = b;
}

uint32_t BlockHeader::nonce() const
{
    return nonce_;
}

void BlockHeader::set_nonce(uint32_t n)
{
    nonce_ = n;
}

Block::Block()
{
    Clear();
}
    
Block::Block(const std::vector<Transaction>& transactions)
    : header_(BlockHeader()), transactions_(transactions) 
{
}

Block::Block(std::vector<Transaction>&& transactions) noexcept
    : header_(BlockHeader()), transactions_(std::move(transactions))
{
}
    
Block::Block(const BlockHeader& header, const std::vector<Transaction>& transactions)
    : header_(header), transactions_(transactions)
{
}

Block::Block(BlockHeader&& header, std::vector<Transaction>&& transactions) noexcept
    : header_(std::move(header)), transactions_(std::move(transactions))
{
}
    
Block::Block(const Block& b)
    : header_(b.header_), transactions_(b.transactions_) 
{
}

Block::Block(Block&& b) noexcept
    : header_(std::move(b.header_)), transactions_(std::move(b.transactions_))
{
}

void Block::Clear()
{
    header_.Clear();
    transactions_.clear();
}

util::Hash256 Block::GetHash() const
{
    return header_.GetHash();
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
    util::Hash256 hash = header_.GetHash();
    util::Hash256 hash_prev_ = header_.hashPrevBlock();
    util::Hash256 hash_merkle = header_.hashMerkleRoot();
    ss << "Block(hash=" << util::EncodeHex(hash.rbegin(), hash.rend()).substr(0,12) << ", "
       << "ver=" << "0x" << std::hex << std::setw(8) << std::setfill('0') << header_.version() << ", "
       << "hashPrevBlock=" << util::EncodeHex(hash_prev_.rbegin(), hash_prev_.rend()).substr(0,12) << ", "
       << "hashMerkleRoot=" << util::EncodeHex(hash_merkle.rbegin(), hash_merkle.rend()).substr(0,12) << ", "
       << "nTime=" << std::dec << header_.time() << ", "
       << "bits=" << std::hex << std::setw(8) << std::setfill('0') << header_.bits() << ", "
       << "nNonce=" << std::dec << header_.nonce() << ", "
       << "tx.size=" << transactions_.size() << ")\n";

    for (const auto& tx : transactions_) {
        ss << "  " << tx.ToString() << "\n";
    }
    
    return ss.str();
}

util::Hash256 Block::ComputeMerkleRoot() const
{
    std::vector<util::Hash256> leaves;
    std::vector<util::Hash256> swap;
    for_each(transactions_.begin(), transactions_.end(), [&leaves](const Transaction& tx)
                                                         { leaves.push_back(tx.GetHash()); });
    
    swap.reserve((leaves.size() + 1) / 2);
    while (leaves.size() > 1) {
        // If number of hashes is odd, duplicate last hash in the list.
        if (leaves.size() % 2 != 0)
            leaves.push_back(leaves.back());
        for (auto it = leaves.begin(); it != leaves.end(); it += 2) {
            swap.push_back(crypto::hashfuncs::DoubleSha256(
                               util::EncodeHex(it[0].rbegin(), it[0].rend()) + 
                               util::EncodeHex(it[1].rbegin(), it[1].rend())));
        }
        std::swap(leaves, swap);
        swap.clear();
    }
    
    // There is now only one item in the list.
    return leaves.front();
}

const BlockHeader& Block::header() const
{
    return header_;
}

void Block::set_header(const BlockHeader& header)
{
    header_ = header;
}

void Block::set_header(BlockHeader&& header)
{
    header_ = std::move(header);
}

const std::vector<Transaction>& Block::transactions() const
{
    return transactions_;
}

void Block::set_transactions(const std::vector<Transaction>& transactions)
{
    transactions_ = transactions;
}

void Block::set_transactions(std::vector<Transaction>&& transactions)
{
    transactions_ = std::move(transactions);
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
    TxIn input(std::move(OutPoint()), script);
    inputs.push_back(input);

    std::vector<TxOut> outputs;
    outputs.push_back(TxOut(reward, output_script));

    std::vector<Transaction> transactions;
    Transaction tx_new(1, inputs, outputs, 0);
    transactions.push_back(tx_new);

    Block genesis(transactions);
    BlockHeader header(version, util::Hash256(), genesis.ComputeMerkleRoot(), time, bits, nonce);    
    genesis.set_header(header);
    
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

} // namespace consensus
} // namespace btclite
