#ifndef BTCLITE_CONSENSUS_PARAMS_H
#define BTCLITE_CONSENSUS_PARAMS_H

#include "btcnet.h"
#include "block.h"


namespace btclite {
namespace consensus {

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
    
    void UpdateVersionBitsParameters(Bip9Deployment::Deployment d, 
                                     int64_t start_time, int64_t timeout);
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

/**
* Parameters that influence chain consensus.
*/
class Params {
public:
    explicit Params(BtcNet btcnet);
    
    const Block& GenesisBlock() const;    
    int SubsideHalvingInterval() const;    
    const Bip9Params& Bip9params() const;

private:
    Block genesis_;
    int subsidy_halving_interval_; // blocks
    Bip9Params bip9_params_;
    //PowParams pow_params_;
    
    void CreateGenesisBlock(const std::string& coinbase, const Script& output_script, 
                            uint32_t time, uint32_t nonce, uint32_t bits, 
                            int32_t version, uint64_t reward);
    void CreateGenesisBlock(uint32_t time, uint32_t nonce, uint32_t bits, 
                            int32_t version, uint64_t reward);
};


} // namespace consensus
} // namespace btclite

#endif // BTCLITE_CONSENSUS_PARAMS_H
