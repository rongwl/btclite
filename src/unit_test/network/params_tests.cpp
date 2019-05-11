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
    EXPECT_TRUE(network_params2.Init(BaseEnv::testnet));
    EXPECT_EQ(testnet_magic, network_params2.msg_magic());
    EXPECT_EQ(18333, network_params2.default_port());    
    std::vector<Seed> vec2 = { { "testnet-seed.bitcoin.jonasschnelli.ch", 18333 },
                              { "seed.tbtc.petertodd.org", 18333 },
                              { "seed.testnet.bitcoin.sprovoost.nl", 18333 },
                              { "testnet-seed.bluematt.me", 18333 } };
    EXPECT_EQ(vec2, network_params2.seeds());
    
    Network::Params network_params3;
    EXPECT_TRUE(network_params3.Init(BaseEnv::regtest));
    EXPECT_EQ(regtest_magic, network_params3.msg_magic());
    EXPECT_EQ(18444, network_params3.default_port());
    EXPECT_EQ(0, network_params3.seeds().size());
    
    Network::Params network_params4;
    EXPECT_FALSE(network_params4.Init(BaseEnv::none));
    EXPECT_EQ(0, network_params4.msg_magic());
    EXPECT_EQ(0, network_params4.default_port());
    EXPECT_EQ(0, network_params4.seeds().size());
}
