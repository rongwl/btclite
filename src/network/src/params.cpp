#include "network/include/params.h"
#include "constants.h"


Network::Params::Params(BaseEnv env)
{
    switch (env) {
        case BaseEnv::mainnet :
        {
            msg_magic_ = kMainMagic;
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

            break;
        }
        case BaseEnv::testnet :
        {
            msg_magic_ = kTestnetMagic;
            default_port_ = 18333;
            seeds_.reserve(4);
            seeds_.push_back({ "testnet-seed.bitcoin.jonasschnelli.ch", 18333 });
            seeds_.push_back({ "seed.tbtc.petertodd.org", 18333 });
            seeds_.push_back({ "seed.testnet.bitcoin.sprovoost.nl", 18333 });
            seeds_.push_back({ "testnet-seed.bluematt.me", 18333});
            
            break;
        }
        case BaseEnv::regtest : 
        {
            msg_magic_ = kRegtestMagic;
            default_port_ = 18444;
            seeds_.clear(); // Regtest mode doesn't have any DNS seeds.
            
            break;
        }
        default :
            msg_magic_ = 0;
            default_port_ = 0;
            seeds_.clear();
            
            break;
    }
}
