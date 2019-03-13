#ifndef BTCLITE_COMMON_PARAMS_H
#define BTCLITE_COMMON_PARAMS_H

#include "block.h"
#include "consensus/include/params.h"
#include "environment.h"
#include "hash.h"


using CheckPoint = std::map<uint32_t, Hash256>;

/*
 * Holds various statistics on transactions within a chain.
 * Used to estimate verification progress during chain sync.
*/
struct ChainTxData {
    int64_t nTime;
    int64_t nTxCount;
    double dTxRate;
};

namespace Common {

/**
 * ChainParams defines various tweakable parameters of a given instance of the
 * Bitcoin system. There are three: the main network on which people trade goods
 * and services, the public test network which gets reset from time to time and
 * a regression test mode which is intended for private networks only. It has
 * minimal difficulty to ensure that blocks can be found instantly.
 */
class Params
{
public:
    enum Base58Type {
        PUBKEY_ADDRESS,
        SCRIPT_ADDRESS,
        SECRET_KEY,
        EXT_PUBLIC_KEY,
        EXT_SECRET_KEY,

        MAX_BASE58_TYPES
    };

	explicit Params(Environment);

    const Consensus::Params& Consensus() const
	{ 
		return consensus_;
	}

    const std::vector<unsigned char>& Base58Prefix(Base58Type type) const
	{
		return base58_prefixes_[type];
	}
	
    const std::string& Bech32HRP() const
	{
		return bech32_hrp_;
	}
	
    const CheckPoint& Checkpoints() const
	{
		return checkpoints_;
	}
	
    const ChainTxData& TxData() const
	{
		return chain_tx_data_;
	}
	
    void UpdateVersionBitsParameters(Bip9Deployment::Deployment, int64_t, int64_t);
	
private:
    Consensus::Params consensus_;
    std::vector<unsigned char> base58_prefixes_[MAX_BASE58_TYPES];
    std::string bech32_hrp_;
    CheckPoint checkpoints_;
    ChainTxData chain_tx_data_;
};

} // namespace Chain

#endif // BTCLITE_COMMON_PARAMS_H
