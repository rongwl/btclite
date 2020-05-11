#include "net_tests.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


namespace btclite {
namespace unit_test {

using namespace network;

TEST_F(LocalServiceTest, Constructor)
{
    EXPECT_EQ(service_.service(), kNodeNetwork | kNodeBloom | kNodeNetworkLimited);
}

TEST_F(LocalServiceTest, GetAndSetLocalServiecs)
{
    service_.set_services(kNodeNetwork);
    EXPECT_EQ(service_.service(), kNodeNetwork);
}

TEST_F(LocalServiceTest, ValidateLocalAddrs)
{
    NetAddr addr;
    
    auto addrs = service_.local_addrs();
    for (auto it = addrs.begin(); it != addrs.end(); ++it)
        EXPECT_TRUE(service_.IsLocal(*it));
    
    addr.SetIpv4(inet_addr("1.2.3.4"));
    EXPECT_FALSE(service_.IsLocal(addr));  
}

TEST_F(LocalServiceTest, GetLocalAddr)
{
    NetAddr peer_addr, addr;
    
    if (service_.local_addrs().empty())
        return;

    peer_addr.SetIpv4(inet_addr("1.2.3.4"));
    ASSERT_TRUE(service_.GetLocalAddr(peer_addr, kNodeNetwork, &addr));
    EXPECT_TRUE(service_.IsLocal(addr));
    EXPECT_EQ(addr.services(), kNodeNetwork);
}


} // namespace unit_test
} // namespace btclit
