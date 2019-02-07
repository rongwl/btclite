#include <memory>
#include <sstream>

#include "block.h"

const Hash256& BlockHeader::Hash() const
{
	if (hash_cache_.IsNull()) {
		std::stringstream ss;
		Serialize(ss);
		DoubleSha256(ss.str(), &hash_cache_);
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
	hash_cache_.SetNull();
	
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
		hash_cache_.SetNull();
	}
	
	return *this;
}

std::string Block::ToString() const
{
    std::stringstream s;
    s << strprintf("Block(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, \
				   nTime=%u, nBits=%08x, nNonce=%u, vtx=%u)\n",
				   header_.Hash().ToString(), header_.version(),
				   header_.hashPrevBlock().ToString(),
				   header_.hashMerkleRoot().ToString(),
				   header_.time(), header_.bits(), header_.nonce(),
				   transactions_.size());
    for (const auto& tx : transactions_) {
        s << "  " << tx.ToString() << "\n";
    }
	
    return s.str();
}

Hash256 Block::ComputeMerkleRoot() const
{
	std::vector<Hash256> leaves;
	std::vector<Hash256> swap;
	for_each(transactions_.begin(), transactions_.end(), [&leaves](const Transaction& tx)
	                                                     { leaves.push_back(tx.Hash()); });
	
	swap.reserve((leaves.size() + 1) / 2);
	while (leaves.size() > 1) {
		// If number of hashes is odd, duplicate last hash in the list.
		if (leaves.size() % 2 != 0)
			leaves.push_back(leaves.back());
		for (auto it = leaves.begin(); it != leaves.end(); it += 2) {
			Hash256 hash;
			DoubleSha256(it[0].ToString()+it[1].ToString(), &hash);
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
						 const uint64_t& reward)
{
    Script script;	
	script.Push(0x1e03ffff);
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
	BlockHeader header(version, Hash256(), genesis.ComputeMerkleRoot(), time, bits, nonce);	
	genesis.set_header(std::move(header));
	
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
Block CreateGenesisBlock(uint32_t time, uint32_t nonce, uint32_t bits, int32_t version, const uint64_t& reward)
{
    const std::string coinbase = "The Times 03/Jan/2009 Chancellor on brink of second bailout for banks";
	Script output_script;
	std::vector<uint8_t> v;
	HexDecode("5F1DF16B2B704C8A578D0BBAF74D385CDE12C11EE50455F3C438EF4C3FBCF649B6DE611FEAE06279A60939E028A8D65C10B73071A6F16719274855FEB0FD8A6704", &v);
	output_script.Push(v);
	output_script.Push(Opcode::OP_CHECKSIG);
    //const CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
    return CreateGenesisBlock(coinbase, output_script, time, nonce, bits, version, reward);
}
