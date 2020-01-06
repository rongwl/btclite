#include <gtest/gtest.h>

#include "fullnode/include/config.h"
#include "p2p.h"


namespace btclite {
namespace unit_test {

TEST(P2PTest, MethodInitArgs)
{
    /*P2P p2p;
    TestArgs args;
    
    p2p.InitArgs(args);
    ASSERT_TRUE(p2p.network_args().is_listen_);
    ASSERT_TRUE(p2p.network_args().is_discover_);
    ASSERT_TRUE(p2p.network_args().is_dnsseed_);
    ASSERT_EQ(p2p.network_args().specified_outgoing_.size(), 0);
    
    
    args.SetArg(FULLNODE_OPTION_LISTEN, "0");
    args.SetArg(FULLNODE_OPTION_DISCOVER, "0");
    args.SetArg(FULLNODE_OPTION_DNSSEED, "0");
    std::vector<std::string> ip = { "1.1.1.1", "2.2.2.2" };
    args.SetArgs(FULLNODE_OPTION_CONNECT, ip[0]);
    args.SetArgs(FULLNODE_OPTION_CONNECT, ip[1]);
    p2p.InitArgs(args);
    EXPECT_FALSE(p2p.network_args().is_listen_);
    EXPECT_FALSE(p2p.network_args().is_discover_);
    EXPECT_FALSE(p2p.network_args().is_dnsseed_);
    EXPECT_EQ(p2p.network_args().specified_outgoing_, ip);*/
}

} // namespace unit_test
} // namespace btclit
