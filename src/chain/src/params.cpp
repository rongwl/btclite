#include "chain/include/params.h"

void Bip9Params::UpdateVersionBitsParameters(Bip9Deployment::Deployment d, int64_t start_time, int64_t timeout)
{

}

void Consensus::Params::Init(NetworkEnv env)
{

}

void Consensus::Params::CreateGenesisBlock(const std::string& coinbase, const Script& output_script,
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

	genesis_.set_transactions(std::move(transactions));
	BlockHeader header(version, Hash256(), genesis_.ComputeMerkleRoot(), time, bits, nonce);	
	genesis_.set_header(std::move(header));
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * Block(hash=0000000000, ver=0x00000001, hashPrevBlock=0000000000, hashMerkleRoot=4a5e1e4baa, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, tx.size=1)
 *   Transaction(hash=4a5e1e4baa, ver=1, inputs.size=1, outputs.size=1, lock_time=0)
 *     TxIn(OutPoint(0000000000, 4294967295), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     TxOut(value=50.00000000, scriptPubKey=4104678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5fac)
 *   vMerkleTree: 4a5e1e
 */
void Consensus::Params::CreateGenesisBlock(uint32_t time, uint32_t nonce, uint32_t bits, int32_t version, uint64_t reward)
{
    const std::string coinbase = "The Times 03/Jan/2009 Chancellor on brink of second bailout for banks";
	Script output_script;
	std::vector<uint8_t> v;
	HexDecode("5F1DF16B2B704C8A578D0BBAF74D385CDE12C11EE50455F3C438EF4C3FBCF649B6DE611FEAE06279A60939E028A8D65C10B73071A6F16719274855FEB0FD8A6704", &v);
	output_script.Push(v);
	output_script.Push(Opcode::OP_CHECKSIG);

    CreateGenesisBlock(coinbase, output_script, time, nonce, bits, version, reward);
}

void Chain::Params::Init(NetworkEnv env)
{
	consensus_.Init(env);
}
