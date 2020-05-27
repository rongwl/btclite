#include "connector_tests.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


namespace btclite {
namespace unit_test {

using namespace network;

TEST_F(ConnectorTest, ConnectNode)
{    
    // init libevent
    ASSERT_TRUE(connector_.InitEvent());
    
    // valid addr
    addrs_.emplace_back(NetAddr());
    EXPECT_FALSE(connector_.ConnectNodes(addrs_, ctx_));
    
    // local addr
    addrs_[0].SetIpv4(inet_addr("1.2.3.4"));
    LocalService local_service2(addrs_);
    ctx_.plocal_service = &local_service2;
    EXPECT_FALSE(connector_.ConnectNodes(addrs_, ctx_));
    
    // banned addr
    banlist_.Add(addrs_[0], BanList::BanReason::kNodeMisbehaving);
    ctx_.plocal_service = &local_service_;
    EXPECT_FALSE(connector_.ConnectNodes(addrs_, ctx_));
    
    LookupHost(std::string("x9.seed.tbtc.petertodd.org").c_str(), &addrs_[0], true, 18333);
    EXPECT_TRUE(connector_.ConnectNodes(addrs_, ctx_));
    
    // exist addr
    EXPECT_FALSE(connector_.ConnectNodes(addrs_, ctx_));
}

TEST_F(ConnectorTest, GetHostAddr)
{
    NetAddr addr;
    
    ASSERT_TRUE(connector_.GetHostAddr("1.2.3.1", 18333, &addr));
    EXPECT_EQ(addr.GetIpv4(), inet_addr("1.2.3.1"));
    
    // node is exist
    auto& nodes = const_cast<Nodes&>(connector_.outbounds());
    nodes.AddNode(std::make_shared<Node>(nullptr, addr));
    ASSERT_FALSE(connector_.GetHostAddr("1.2.3.1", 18333, &addr));
    EXPECT_TRUE(connector_.GetHostAddr("1.2.3.2", 18333, &addr));
    
    EXPECT_TRUE(connector_.GetHostAddr("bitcoin.org", 18333, &addr));
}

TEST_F(ConnectorTest, ConnectOutbound)
{
    NetAddr addr, source;
    
    // init libevent
    ASSERT_TRUE(connector_.InitEvent());
    
    // peers is null
    EXPECT_FALSE(connector_.OutboundTimeOutCb(ctx_));

    // connecting peer is local
    addrs_.emplace_back(NetAddr());
    addrs_[0].SetIpv4(inet_addr("1.2.3.4"));
    LocalService local_service2(addrs_);
    ctx_.plocal_service = &local_service2;
    source.SetIpv4(inet_addr("1.2.3.5"));
    peers_.Add(addrs_[0], source);
    EXPECT_FALSE(connector_.OutboundTimeOutCb(ctx_));
    peers_.Clear();
  
    LookupHost(std::string("x9.seed.tbtc.petertodd.org").c_str(), &addr, true, 18333);
    addr.set_port(18333);
    addr.set_services(kDesirableServiceFlags);
    peers_.Add(addr, source);
    ctx_.plocal_service = &local_service_;
    EXPECT_TRUE(connector_.OutboundTimeOutCb(ctx_));

}

TEST_F(ConnectorTest, DnsLookup)
{    
    ASSERT_TRUE(connector_.DnsLookup(params_.seeds(), 18333, &peers_));
    EXPECT_FALSE(peers_.IsEmpty());
}

} // namespace unit_test
} // namespace btclit
