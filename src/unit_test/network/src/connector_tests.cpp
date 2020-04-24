#include <gtest/gtest.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "connector.h"
#include "bandb.h"
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
    
    // init libevent
    ASSERT_TRUE(connector.InitEvent());
    
    // valid addr
    addrs.emplace_back(NetAddr());
    EXPECT_FALSE(connector.ConnectNodes(addrs));
    
    // local addr
    LocalService& service = SingletonLocalService::GetInstance();
    if (!service.local_addrs().empty()) {
        addrs[0] = service.local_addrs().front();
        EXPECT_FALSE(connector.ConnectNodes(addrs));
    }
    
    // banned addr
    addrs[0].SetIpv4(inet_addr("1.2.3.4"));
    SingletonBanList::GetInstance().Add(addrs[0], BanList::BanReason::kNodeMisbehaving);
    EXPECT_FALSE(connector.ConnectNodes(addrs));
    
    LookupHost(std::string("x9.seed.tbtc.petertodd.org").c_str(), &addrs[0], true, 18333);
    EXPECT_TRUE(connector.ConnectNodes(addrs));
    
    // exist addr
    EXPECT_FALSE(connector.ConnectNodes(addrs));
    
    SingletonBanList::GetInstance().Clear();
    SingletonNodes::GetInstance().Clear();
}

TEST(ConnectorTest, GetHostAddr)
{
    network::Params params(BtcNet::kTestNet, util::Args(), fs::path("/tmp/foo"));
    Connector connector(params);
    NetAddr addr;
    
    ASSERT_TRUE(connector.GetHostAddr("1.2.3.1", &addr));
    EXPECT_EQ(addr.GetIpv4(), inet_addr("1.2.3.1"));
    
    // node is exist
    SingletonNodes::GetInstance().AddNode(std::make_shared<Node>(nullptr, addr));
    ASSERT_FALSE(connector.GetHostAddr("1.2.3.1", &addr));
    EXPECT_TRUE(connector.GetHostAddr("1.2.3.2", &addr));
    
    ASSERT_TRUE(connector.GetHostAddr("bitcoin.org", &addr));
    
    SingletonNodes::GetInstance().Clear();
}

TEST(ConnectorTest, ConnectOutbound)
{
    network::Params params(BtcNet::kTestNet, util::Args(), fs::path("/tmp/foo"));
    Connector connector(params);
    NetAddr addr, source;
    
    // init libevent
    ASSERT_TRUE(connector.InitEvent());
    
    // peers is null
    ASSERT_FALSE(connector.OutboundTimeOutCb());
    
    // connecting peer is local
    LocalService& service = SingletonLocalService::GetInstance();
    if (!service.local_addrs().empty()) {
        source.SetIpv4(inet_addr("1.2.3.4"));
        SingletonPeers::GetInstance().Add(service.local_addrs().front(), source);
        ASSERT_FALSE(connector.OutboundTimeOutCb());
        SingletonPeers::GetInstance().Clear();
    }
    
    LookupHost(std::string("x9.seed.tbtc.petertodd.org").c_str(), &addr, true, 18333);
    addr.set_port(8333);
    addr.set_services(kDesirableServiceFlags);
    SingletonPeers::GetInstance().Add(addr, source);
    EXPECT_TRUE(connector.OutboundTimeOutCb());

    SingletonPeers::GetInstance().Clear();
}

TEST(ConnectorTest, DnsLookup)
{
    network::Params params(BtcNet::kTestNet, util::Args(), fs::path("/tmp/foo"));
    ASSERT_TRUE(SingletonPeers::GetInstance().IsEmpty());
    ASSERT_TRUE(Connector::DnsLookup(params.seeds(), 18333));
    EXPECT_FALSE(SingletonPeers::GetInstance().IsEmpty());
    SingletonPeers::GetInstance().Clear();
}

} // namespace unit_test
} // namespace btclit
