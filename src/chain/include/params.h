#ifndef BTCLITE_CHAIN_PARAMS_H
#define BTCLITE_CHAIN_PARAMS_H

#include <cstdint>

#include "block.h"
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

struct Bip9Deployment {
   enum Deployment {
       TESTDUMMY,
       CSV, // Deployment of BIP68, BIP112, and BIP113.
       SEGWIT, // Deployment of BIP141, BIP143, and BIP147.
       
       MAX_DEPLOYMENTS
   };
   
   /** Bit position to select the particular bit in nVersion. */
   int bit;
   /** Start MedianTime for version bits miner confirmation. Can be a date in the past */
   int64_t start_time;
   /** Timeout/expiry MedianTime for the deployment attempt. */
   int64_t timeout;

   /** Constant for nTimeout very far in the future. */
   static constexpr int64_t NO_TIMEOUT = std::numeric_limits<int64_t>::max();

   /** Special value for nStartTime indicating that the deployment is always active.
    *  This is useful for testing, as it means tests don't need to deal with the activation
    *  process (which takes at least 3 BIP9 intervals). Only tests that specifically test the
    *  behaviour during activation cannot use this. */
   static constexpr int64_t ALWAYS_ACTIVE = -1;
};

/**
* Minimum blocks including miner confirmation of the total of 2016 blocks in a retargeting period,
* (nPowTargetTimespan / nPowTargetSpacing) which is also used for BIP9 deployments.
* Examples: 1916 for 95%, 1512 for testchains.
*/
struct Bip9Params {
   uint32_t bip9_activation_threshold;
   uint32_t miner_confirmation_window;
   Bip9Deployment deployments[Bip9Deployment::MAX_DEPLOYMENTS];
   
   void UpdateVersionBitsParameters(Bip9Deployment::Deployment d, int64_t start_time, int64_t timeout);
};

/*
struct PowParams {
uint256 powLimit;
   bool fPowAllowMinDifficultyBlocks;
   bool fPowNoRetargeting;
   int64_t nPowTargetSpacing;
   int64_t nPowTargetTimespan;
   uint256 nMinimumChainWork;
   uint256 defaultAssumeValid;
 
int64_t DifficultyAdjustmentInterval() const
{
 return nPowTargetTimespan / nPowTargetSpacing;
}
};
*/

namespace Consensus {

/**
* Parameters that influence chain consensus.
*/
class Params {
public:
    Params()
        : genesis_(), subsidy_halving_interval_(0), bip9_params_() {}
    
    void Init(NetworkEnv env);
    
    const Block& GenesisBlock() const
    {
        return genesis_;
    }
    
    int SubsideHalvingInterval() const
    {
        return subsidy_halving_interval_;
    }
    
    const Bip9Params& Bip9params() const
    {
        return bip9_params_;
    }

private:
    Block genesis_;
    int subsidy_halving_interval_;
    Bip9Params bip9_params_;
    //PowParams pow_params_;
    
    void CreateGenesisBlock(const std::string& coinbase, const Script& output_script, uint32_t time,
                             uint32_t nonce, uint32_t bits, int32_t version, uint64_t reward);
    void CreateGenesisBlock(uint32_t time, uint32_t nonce, uint32_t bits, int32_t version, uint64_t reward);
};

} // namespace Consensus


namespace Chain {

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
    
    Params()
        : consensus_(), prune_after_height_(0), base58_prefixes_(),
          bech32_hrp_(), checkpoints_(), chain_tx_data_() {}

    //-------------------------------------------------------------------------
    void Init(NetworkEnv env);

    //-------------------------------------------------------------------------
    const Consensus::Params& consensus() const
    { 
        return consensus_;
    }
    
    uint64_t prune_after_height() const
    {
        return prune_after_height_;
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
    Consensus::Params consensus_;
    uint64_t prune_after_height_;
    std::vector<unsigned char> base58_prefixes_[MAX_BASE58_TYPES];
    std::string bech32_hrp_;
    CheckPoint checkpoints_;
    ChainTxData chain_tx_data_;
};

} // namespace Chain


#endif // BTCLITE_CHAIN_PARAMS_H
