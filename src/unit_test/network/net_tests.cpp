#include <gtest/gtest.h>
#include <cstdint>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "net.h"
#include "constants.h"
#include "network/include/params.h"


TEST(LocalNetConfigTest, Constructor)
{
    LocalNetConfig config;
    EXPECT_EQ(config.local_services(), kNodeNetwork | kNodeNetworkLimited);
}

TEST(LocalNetConfigTest, GetAndSetLocalServiecs)
{
    LocalNetConfig config;
    config.set_local_services(kNodeNetwork);
    EXPECT_EQ(config.local_services(), kNodeNetwork);
}

TEST(LocalNetConfigTest, ValidateLocalAddrs)
{
    LocalNetConfig config;
    btclite::network::NetAddr addr;
    
    ASSERT_TRUE(config.LookupLocalAddrs());
    EXPECT_TRUE(config.IsLocal(config.local_addrs().front()));
    EXPECT_TRUE(config.IsLocal(config.local_addrs().back()));
    
    addr.SetIpv4(inet_addr("1.2.3.4"));
    EXPECT_FALSE(config.IsLocal(addr));  
}
