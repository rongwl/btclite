#include "network/include/params.h"
#include "constants.h"
#include "fullnode/include/config.h"


namespace btclite {
namespace network {

bool Seed::operator==(const Seed& b) const
{
    return (host == b.host && port == b.port);
}

bool Seed::operator!=(const Seed& b) const
{
    return !(*this == b);
}

Params::Params(const util::Configuration& config)
    : Params(config.btcnet(), config.args(), config.path_data_dir())
{
}

Params::Params(BtcNet btcnet, const util::Args& args, 
               const fs::path& path_data_dir)
    : path_data_dir_(path_data_dir)
{
    if (args.IsArgSet(FULLNODE_OPTION_CONNECT)) {
        advertise_local_addr_ = false;
        discover_local_addr_ = false;
        use_dnsseed_ = false;
        specified_outgoing_ = args.GetArgs(FULLNODE_OPTION_CONNECT);
    }
    
    switch (btcnet) {
        case BtcNet::kMainNet :
        {
            msg_magic_ = kMainnetMagic;
            default_port_ = 8333;
            seeds_.reserve(8);
            seeds_.push_back({ "seed.bitcoin.sipa.be", 8333 }); // Pieter Wuille, only supports x1, x5, x9, and xd
            seeds_.push_back({ "dnsseed.bluematt.me", 8333 }); // Matt Corallo, only supports x9
            seeds_.push_back({ "dnsseed.bitcoin.dashjr.org", 8333 }); // Luke Dashjr
            seeds_.push_back({ "seed.bitcoinstats.com", 8333 }); // Christian Decker, supports x1 - xf
            seeds_.push_back({ "seed.bitcoin.jonasschnelli.ch", 8333 }); // Jonas Schnelli, only supports x1, x5, x9, and xd
            seeds_.push_back({ "seed.btc.petertodd.org", 8333 }); // Peter Todd, only supports x1, x5, x9, and xd
            seeds_.push_back({ "seed.bitcoin.sprovoost.nl", 8333 }); // Sjors Provoost
            seeds_.push_back({ "dnsseed.emzy.de", 8333 }); // Stephan Oeste
            BTCLOG(LOG_LEVEL_VERBOSE) << "mainnet params: msg_magic=" << std::hex << msg_magic_
                                      << " default_port=" << std::dec << default_port_;

            break;
        }
        case BtcNet::kTestNet :
        {
            msg_magic_ = kTestnetMagic;
            default_port_ = 18333;
            seeds_.reserve(4);
            seeds_.push_back({ "testnet-seed.bitcoin.jonasschnelli.ch", 18333 });
            seeds_.push_back({ "seed.tbtc.petertodd.org", 18333 });
            seeds_.push_back({ "seed.testnet.bitcoin.sprovoost.nl", 18333 });
            seeds_.push_back({ "testnet-seed.bluematt.me", 18333});
            BTCLOG(LOG_LEVEL_VERBOSE) << "testnet params: msg_magic=" << std::hex << msg_magic_
                                      << " default_port=" << std::dec << default_port_;
            
            break;
        }
        case BtcNet::kRegTest : 
        {
            msg_magic_ = kRegtestMagic;
            default_port_ = 18444;
            seeds_.clear(); // Regtest mode doesn't have any DNS seeds.
            BTCLOG(LOG_LEVEL_VERBOSE) << "regtest params: msg_magic=" << std::hex << msg_magic_
                                      << " default_port=" << std::dec << default_port_;
            
            break;
        }
        default :
        {
            msg_magic_ = 0;
            default_port_ = 0;
            seeds_.clear();
            
            break;
        }
    }
}

uint32_t Params::msg_magic() const
{
    return msg_magic_;
}

uint16_t Params::default_port() const
{
    return default_port_;
}

const std::vector<Seed>& Params::seeds() const
{
    return seeds_;
}

bool Params::advertise_local_addr() const
{
    return advertise_local_addr_;
}

bool Params::discover_local_addr() const
{
    return discover_local_addr_;
}

bool Params::use_dnsseed() const
{
    return use_dnsseed_;
}

const std::vector<std::string>& Params::specified_outgoing() const
{
    return specified_outgoing_;
}

const fs::path& Params::path_data_dir() const
{
    return path_data_dir_;
}

} // namespace network
} // namespace btclite
