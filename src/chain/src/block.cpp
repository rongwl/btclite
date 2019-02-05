#include <memory>
#include <sstream>

#include "block.h"

const Hash256& BlockHeader::Hash()
{
	std::stringstream ss;
	Serialize(ss);
	DoubleSha256(ss.str(), &hash_);
	
	return hash_;
}

std::string Block::ToString() const
{
    std::stringstream s;
    s << strprintf("Block(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, \
				   nTime=%u, nBits=%08x, nNonce=%u, vtx=%u)\n",
				   header_.HashCache().ToString(), header_.version(),
				   header_.hashPrevBlock().ToString(),
				   header_.hashMerkleRoot().ToString(),
				   header_.time(), header_.bits(), header_.nonce(),
				   transactions_.size());
    for (const auto& tx : transactions_) {
        s << "  " << tx.ToString() << "\n";
    }
	
    return s.str();
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
	
	BlockHeader header(version, Hash256(), Hash256(), time, bits, nonce);
	//header.set_hashMerkleRoot();
	
    Block genesis(std::move(header), std::move(transactions));
	
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
/*Block CreateGenesisBlock(uint32_t time, uint32_t nonce, uint32_t bits, int32_t version, const uint64_t& reward)
{
    const std::string coinbase = "Shanghai Daily 01/Aug/2018 China stresses economic stability";
    const CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}*/
