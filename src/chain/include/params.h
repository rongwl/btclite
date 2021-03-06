#ifndef BTCLITE_CHAIN_PARAMS_H
#define BTCLITE_CHAIN_PARAMS_H


#include "consensus/include/params.h"


namespace btclite {
namespace chain {

using CheckPoint = std::map<uint32_t, util::Hash256>;

/*
 * Holds various statistics on transactions within a chain.
 * Used to estimate verification progress during chain sync.
*/
struct ChainTxData {
    // UNIX timestamp of last known number of transactions
    int64_t timestamp;
    // total number of transactions between genesis and that timestamp
    int64_t tx_count;
    //estimated number of transactions per second after that timestamp
    double tx_rate;
};

/*
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
    
    explicit Params(BtcNet btcnet);

    //-------------------------------------------------------------------------
    const consensus::Params& consensus_params() const
    { 
        return consensus_params_;
    }

    const std::vector<unsigned char>& base58_prefix(Base58Type type) const
    {
        return base58_prefixes_[type];
    }
    
    const std::string& bech32_hrp() const
    {
        return bech32_hrp_;
    }
    
    const CheckPoint& checkpoints() const
    {
        return checkpoints_;
    }
    
    const ChainTxData& chain_tx_data() const
    {
        return chain_tx_data_;
    }
    
private:
    consensus::Params consensus_params_;
    std::vector<unsigned char> base58_prefixes_[MAX_BASE58_TYPES];
    std::string bech32_hrp_;
    CheckPoint checkpoints_;
    ChainTxData chain_tx_data_;
};

} // namespace chain
} // namespace btclite

#endif // BTCLITE_CHAIN_PARAMS_H
