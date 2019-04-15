#include "network/include/params.h"
#include "constants.h"

void Network::Params::Init(NetworkEnv env)
{
    switch (env) {
        case NetworkEnv::mainnet :
        {
            msg_magic_ = main_magic;
            default_port_ = 8333;
            seeds_.push_back({ "seed.bitcoin.sipa.be", 8333 }); // Pieter Wuille, only supports x1, x5, x9, and xd
            seeds_.push_back({ "dnsseed.bluematt.me", 8333 }); // Matt Corallo, only supports x9
            seeds_.push_back({ "dnsseed.bitcoin.dashjr.org", 8333 }); // Luke Dashjr
            seeds_.push_back({ "seed.bitcoinstats.com", 8333 }); // Christian Decker, supports x1 - xf
            seeds_.push_back({ "seed.bitcoin.jonasschnelli.ch", 8333 }); // Jonas Schnelli, only supports x1, x5, x9, and xd
            seeds_.push_back({ "seed.btc.petertodd.org", 8333 }); // Peter Todd, only supports x1, x5, x9, and xd
            
            break;
        }
    }
}
