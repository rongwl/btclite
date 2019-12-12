#include "net_tests.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


using namespace btclite::network;

TEST_F(LocalNetConfigTest, Constructor)
{
    EXPECT_EQ(config_.local_services(), kNodeNetwork | kNodeNetworkLimited);
}

TEST_F(LocalNetConfigTest, GetAndSetLocalServiecs)
{
    config_.set_local_services(kNodeNetwork);
    EXPECT_EQ(config_.local_services(), kNodeNetwork);
}

TEST_F(LocalNetConfigTest, ValidateLocalAddrs)
{
    btclite::network::NetAddr addr;
    
    ASSERT_FALSE(config_.map_local_addrs().empty());
    std::map<btclite::network::NetAddr, int> map = config_.map_local_addrs();
    for (auto it = map.begin(); it != map.end(); ++it)
        EXPECT_TRUE(config_.IsLocal(it->first));
    
    addr.SetIpv4(inet_addr("1.2.3.4"));
    EXPECT_FALSE(config_.IsLocal(addr));  
}

TEST_F(LocalNetConfigTest, GetLocalAddr)
{
    btclite::network::NetAddr peer_addr, addr;
    
    ASSERT_FALSE(config_.map_local_addrs().empty());
    peer_addr.SetIpv4(inet_addr("1.2.3.4"));
    ASSERT_TRUE(config_.GetLocalAddr(peer_addr, kNodeNetwork, &addr));
    EXPECT_TRUE(config_.IsLocal(addr));
    EXPECT_EQ(addr.services(), kNodeNetwork);
}
