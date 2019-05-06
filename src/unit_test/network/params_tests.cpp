#include <gtest/gtest.h>
#include <vector>

#include "constants.h"
#include "environment.h"
#include "network/include/params.h"

TEST(NetworkParamsTest, Methord_Init)
{
    Network::Params network_params;
    EXPECT_TRUE(network_params.Init(BaseEnv::mainnet));
    EXPECT_EQ(main_magic, network_params.msg_magic());
    EXPECT_EQ(8333, network_params.default_port());
    
    std::vector<Seed> vec = { { "seed.bitcoin.sipa.be", 8333 },
                              { "dnsseed.bluematt.me", 8333 },
                              { "dnsseed.bitcoin.dashjr.org", 8333 },
                              { "seed.bitcoinstats.com", 8333 },
                              { "seed.bitcoin.jonasschnelli.ch", 8333 },
                              { "seed.btc.petertodd.org", 8333 },
                              { "seed.bitcoin.sprovoost.nl", 8333 },
                              { "dnsseed.emzy.de", 8333 } };
    EXPECT_EQ(vec, network_params.seeds());
    
    Network::Params network_params2;
    EXPECT_FALSE(network_params.Init(BaseEnv::none));
}
