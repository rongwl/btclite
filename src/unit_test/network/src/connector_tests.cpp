#include <gtest/gtest.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "connector.h"
#include "banlist.h"
#include "net.h"
#include "net_base.h"
#include "peers.h"


namespace btclite {
namespace unit_test {

using namespace network;

TEST(ConnectorTest, ConnectNode)
{
    std::vector<NetAddr> addrs;
    network::Params params(BtcNet::kTestNet, util::Args(), fs::path("/tmp/foo"));
    Connector connector(params);
    Peers peers;
    BanList banlist;
    
    // init libevent
    ASSERT_TRUE(connector.InitEvent());
    
    // valid addr
    addrs.emplace_back(NetAddr());
    EXPECT_FALSE(connector.ConnectNodes(addrs, LocalService(), banlist, &peers));
    
    // local addr
    addrs[0].SetIpv4(inet_addr("1.2.3.4"));
    LocalService local_service(addrs);
    EXPECT_FALSE(connector.ConnectNodes(addrs, local_service, banlist, &peers));
    
    // banned addr
    banlist.Add(addrs[0], BanList::BanReason::kNodeMisbehaving);
    EXPECT_FALSE(connector.ConnectNodes(addrs, LocalService(), banlist, &peers));
    
    LookupHost(std::string("x9.seed.tbtc.petertodd.org").c_str(), &addrs[0], true, 18333);
    EXPECT_TRUE(connector.ConnectNodes(addrs, LocalService(), banlist, &peers));
    
    // exist addr
    EXPECT_FALSE(connector.ConnectNodes(addrs, LocalService(), banlist, &peers));
}

TEST(ConnectorTest, GetHostAddr)
{
    network::Params params(BtcNet::kTestNet, util::Args(), fs::path("/tmp/foo"));
    Connector connector(params);
    NetAddr addr;
    
    ASSERT_TRUE(connector.GetHostAddr("1.2.3.1", &addr));
    EXPECT_EQ(addr.GetIpv4(), inet_addr("1.2.3.1"));
    
    // node is exist
    auto& nodes = const_cast<Nodes&>(connector.outbounds());
    nodes.AddNode(std::make_shared<Node>(nullptr, addr));
    ASSERT_FALSE(connector.GetHostAddr("1.2.3.1", &addr));
    EXPECT_TRUE(connector.GetHostAddr("1.2.3.2", &addr));
    
    EXPECT_TRUE(connector.GetHostAddr("bitcoin.org", &addr));
}

TEST(ConnectorTest, ConnectOutbound)
{
    network::Params params(BtcNet::kTestNet, util::Args(), fs::path("/tmp/foo"));
    Connector connector(params);
    NetAddr addr, source;
    std::vector<NetAddr> addrs;
    Peers peers;
    BanList banlist;
    
    // init libevent
    ASSERT_TRUE(connector.InitEvent());
    
    // peers is null
    EXPECT_FALSE(connector.OutboundTimeOutCb(LocalService(), banlist, &peers));

    // connecting peer is local
    addrs.emplace_back(NetAddr());
    addrs[0].SetIpv4(inet_addr("1.2.3.4"));
    LocalService local_service(addrs);
    source.SetIpv4(inet_addr("1.2.3.5"));
    peers.Add(addrs[0], source);
    EXPECT_FALSE(connector.OutboundTimeOutCb(local_service, banlist, &peers));
    peers.Clear();
  
    LookupHost(std::string("x9.seed.tbtc.petertodd.org").c_str(), &addr, true, 18333);
    addr.set_port(8333);
    addr.set_services(kDesirableServiceFlags);
    peers.Add(addr, source);
    EXPECT_TRUE(connector.OutboundTimeOutCb(LocalService(), banlist, &peers));

}

TEST(ConnectorTest, DnsLookup)
{
    network::Params params(BtcNet::kTestNet, util::Args(), fs::path("/tmp/foo"));
    Connector connector(params);
    Peers peers;
    
    ASSERT_TRUE(connector.DnsLookup(params.seeds(), 18333, &peers));
    EXPECT_FALSE(peers.IsEmpty());
}

} // namespace unit_test
} // namespace btclit
