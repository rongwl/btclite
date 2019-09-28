#include <gtest/gtest.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "connector.h"
#include "bandb.h"
#include "netbase.h"
#include "peers.h"


TEST(ConnectorTest, ConnectNode)
{
    std::vector<btclite::network::NetAddr> addrs;
    Connector connector;
    
    // init libevent
    ASSERT_TRUE(connector.InitEvent());
    
    // valid addr
    addrs.emplace_back(btclite::network::NetAddr());
    EXPECT_FALSE(connector.ConnectNodes(addrs));
    
    // local addr
    LocalNetConfig& config = SingletonLocalNetCfg::GetInstance();
    ASSERT_TRUE(config.LookupLocalAddrs());
    addrs[0] = std::move(config.local_addrs().front());
    EXPECT_FALSE(connector.ConnectNodes(addrs));
    
    // banned addr
    addrs[0].SetIpv4(inet_addr("1.2.3.4"));
    SingletonBanDb::GetInstance().Add(addrs[0], BanDb::NodeMisbehaving, false);
    EXPECT_FALSE(connector.ConnectNodes(addrs));
    
    LookupHost(std::string("x9.seed.tbtc.petertodd.org").c_str(), &addrs[0], true);
    EXPECT_TRUE(connector.ConnectNodes(addrs));
    
    // exist addr
    EXPECT_FALSE(connector.ConnectNodes(addrs));
    
    SingletonNodes::GetInstance().Clear();
}

TEST(ConnectorTest, GetHostAddr)
{
    Connector connector;
    btclite::network::NetAddr addr;
    
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
    Connector connector;
    btclite::network::NetAddr addr, source;
    
    // init libevent
    ASSERT_TRUE(connector.InitEvent());
    
    // peers is null
    ASSERT_FALSE(connector.OutboundTimeOutCb());
    
    // connecting peer is local
    LocalNetConfig& config = SingletonLocalNetCfg::GetInstance();
    config.LookupLocalAddrs();
    source.SetIpv4(inet_addr("1.2.3.4"));
    SingletonPeers::GetInstance().Add(config.local_addrs(), source);
    ASSERT_FALSE(connector.OutboundTimeOutCb());
    SingletonPeers::GetInstance().Clear();
    
    LookupHost(std::string("x9.seed.tbtc.petertodd.org").c_str(), &addr, true);
    addr.mutable_proto_addr()->set_port(8333);
    addr.mutable_proto_addr()->set_services(desirable_service_flags);
    SingletonPeers::GetInstance().Add(addr, source);
    EXPECT_TRUE(connector.OutboundTimeOutCb());
    
    SingletonPeers::GetInstance().Clear();
}

TEST(ConnectorTest, DnsLookup)
{
    const std::vector<Seed>& seeds = btclite::network::SingletonParams::GetInstance(BaseEnv::testnet).seeds();
    ASSERT_TRUE(SingletonPeers::GetInstance().IsEmpty());
    ASSERT_TRUE(Connector::DnsLookup(seeds));
    EXPECT_FALSE(SingletonPeers::GetInstance().IsEmpty());
    SingletonPeers::GetInstance().Clear();    
}
