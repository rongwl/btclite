#include "node_tests.h"

#include "banlist.h"


namespace btclite {
namespace unit_test {

using namespace network;

TEST_F(BroadcastAddrsTest, PushAddrToSend)
{
    ASSERT_GT(flooding_addrs_.addrs_to_send().size(), 0);
    EXPECT_EQ(flooding_addrs_.addrs_to_send()[0], sent_addr_);
}

TEST_F(BroadcastAddrsTest, ClearSentAddr)
{
    flooding_addrs_.ClearSentAddr();
    EXPECT_EQ(flooding_addrs_.addrs_to_send().size(), 0);
}

TEST_F(BroadcastAddrsTest, AddKnownAddr)
{
    EXPECT_TRUE(flooding_addrs_.IsKnownAddr(known_addr_));
}

TEST_F(NodesTest, InitializeNode)
{
    auto node3 = nodes_.GetNode(addr3_);
    ASSERT_NE(node3, nullptr);
    EXPECT_EQ(node3->connection().addr(), addr3_);
}

TEST_F(NodesTest, GetNodeByAddr)
{
    auto node1 = nodes_.GetNode(addr1_);
    ASSERT_NE(node1, nullptr);
    EXPECT_EQ(node1->connection().addr(), addr1_);
}

TEST_F(NodesTest, GetNodeById)
{
    auto node2 = nodes_.GetNode(id2_);
    ASSERT_NE(node2, nullptr);
    EXPECT_EQ(node2->connection().addr(), addr2_);
}

TEST_F(NodesTest, GetNodeBySubnet)
{
    NetAddr netmask;
    std::vector<std::shared_ptr<Node> > nodes_vec;
    auto node1 = nodes_.GetNode(id1_);
    auto node2 = nodes_.GetNode(id2_);
    auto node3 = nodes_.GetNode(id3_);
    
    netmask.SetIpv4(inet_addr("255.255.255.0"));
    SubNet subnet(addr1_, netmask);
    nodes_.GetNode(subnet, &nodes_vec);
    
    EXPECT_EQ(node1->connection().addr(), nodes_vec[0]->connection().addr());
    EXPECT_EQ(node2->connection().addr(), nodes_vec[1]->connection().addr());
    EXPECT_EQ(node3->connection().addr(), nodes_vec[2]->connection().addr());
}

TEST_F(NodesTest, EraseNodeByPtr)
{
    auto node1 = nodes_.GetNode(id1_);
    ASSERT_NE(node1, nullptr);
    nodes_.EraseNode(node1);
    EXPECT_EQ(nodes_.GetNode(addr1_), nullptr);
    EXPECT_EQ(nodes_.GetNode(id1_), nullptr);
    EXPECT_NE(nodes_.GetNode(id2_), nullptr);
}

TEST_F(NodesTest, EraseNodeById)
{
    auto node = nodes_.GetNode(id2_);
    ASSERT_NE(node, nullptr);
    nodes_.EraseNode(id2_);
    EXPECT_EQ(nodes_.GetNode(addr2_), nullptr);
    EXPECT_EQ(nodes_.GetNode(id2_), nullptr);
    EXPECT_NE(nodes_.GetNode(id1_), nullptr);
}

TEST_F(NodesTest, CheckIncomingNonce)
{
    auto node1 = nodes_.GetNode(id1_);
    auto node2 = nodes_.GetNode(id2_);
    ASSERT_NE(node1, nullptr);
    ASSERT_NE(node2, nullptr);
    EXPECT_FALSE(nodes_.CheckIncomingNonce(node1->local_host_nonce()));
    EXPECT_TRUE(nodes_.CheckIncomingNonce(1234));
    EXPECT_TRUE(nodes_.CheckIncomingNonce(node2->local_host_nonce()));
    node1->mutable_connection()->set_connection_state(NodeConnection::kEstablished);
    EXPECT_TRUE(nodes_.CheckIncomingNonce(node1->local_host_nonce()));
}

TEST_F(NodesTest, InboundCount)
{
    NetAddr addr;
    
    addr.SetIpv4(inet_addr("1.1.1.4"));
    nodes_.InitializeNode(nullptr, addr, false);
    addr.SetIpv4(inet_addr("1.1.1.5"));
    nodes_.InitializeNode(nullptr, addr, false);
    EXPECT_EQ(nodes_.CountInbound(), 2);
    EXPECT_EQ(nodes_.CountOutbound(), 3);
}

TEST_F(NodesTest, CountSyncStarted)
{
    auto node1 = nodes_.GetNode(id1_);
    auto node2 = nodes_.GetNode(id2_);
    
    EXPECT_EQ(nodes_.CountSyncStarted(), 0);
    node1->mutable_block_sync_state()->set_sync_started(true);
    node2->mutable_block_sync_state()->set_sync_started(true);
    EXPECT_EQ(nodes_.CountSyncStarted(), 2);
}

TEST_F(NodesTest, CountPreferredDownload)
{
    auto node1 = nodes_.GetNode(id1_);
    auto node2 = nodes_.GetNode(id2_);
    
    EXPECT_EQ(nodes_.CountPreferredDownload(), 0);
    node1->mutable_protocol()->set_services(kNodeNetwork);
    EXPECT_EQ(nodes_.CountPreferredDownload(), 1);
}

TEST_F(NodesTest, CountValidatedDownload)
{
    auto node1 = nodes_.GetNode(id1_);
    auto node2 = nodes_.GetNode(id2_);
    
    EXPECT_EQ(nodes_.CountValidatedDownload(), 0);
    node1->mutable_blocks_in_flight()->set_valid_headers_count(1);
    node2->mutable_blocks_in_flight()->set_valid_headers_count(1);
    EXPECT_EQ(nodes_.CountValidatedDownload(), 2);
}

TEST_F(NodeTest, Misbehaving)
{
    auto node = SingletonNodes::GetInstance().GetNode(id_);
    
    node->mutable_misbehavior()->Misbehaving(id_, kDefaultBanscoreThreshold - 1);
    ASSERT_FALSE(node->misbehavior().should_ban());
    node->mutable_misbehavior()->Misbehaving(id_, 1);
    EXPECT_TRUE(node->misbehavior().should_ban());
}

TEST_F(NodeTest, HandleInactiveTimeout)
{
    Nodes& nodes = SingletonNodes::GetInstance();
    std::shared_ptr<Node> node = nodes.GetNode(id_);
    ASSERT_NE(node, nullptr);
    NodeTimeoutCb::InactivityTimeoutCb(node);
    EXPECT_TRUE(node->connection().IsDisconnected());
}

TEST_F(NodeTest, CheckBanned)
{
    Nodes& nodes = SingletonNodes::GetInstance();
    std::shared_ptr<Node> node = nodes.GetNode(id_);
    ASSERT_NE(node, nullptr);
    
    node->mutable_misbehavior()->set_should_ban(true);
    std::cout << node.use_count() << '\n';
    ASSERT_TRUE(node->CheckBanned());
    EXPECT_TRUE(node->connection().IsDisconnected());
    EXPECT_TRUE(SingletonBanList::GetInstance().IsBanned(addr_));

    SingletonBanList::GetInstance().Clear();
}

TEST_F(NodeTest, PushAddrToSend)
{
    Nodes& nodes = SingletonNodes::GetInstance();
    std::shared_ptr<Node> node = nodes.GetNode(id_);
    node->mutable_flooding_addrs()->AddKnownAddr(addr_);
    EXPECT_FALSE(node->mutable_flooding_addrs()->PushAddrToSend(addr_));
    
    addr_.set_port(8333);
    ASSERT_TRUE(node->mutable_flooding_addrs()->PushAddrToSend(addr_));
    EXPECT_EQ(node->flooding_addrs().addrs_to_send().front(), addr_);
}

TEST(DisconnectedNode, DisconnectedNodeByPtr)
{
    Nodes& nodes = SingletonNodes::GetInstance();
    NetAddr addr;
    
    addr.SetIpv4(inet_addr("1.1.1.1"));
    auto node = std::make_shared<Node>(nullptr, addr);
    nodes.AddNode(node);
    
    ASSERT_NE(nodes.GetNode(node->id()), nullptr);
    DisconnectNode(node);
    EXPECT_EQ(nodes.GetNode(node->id()), nullptr);
    
    nodes.Clear();
}

TEST(DisconnectedNode, DisconnectedNodeBySubnet)
{
    Nodes& nodes = SingletonNodes::GetInstance();
    
    NetAddr addr;    
    addr.SetIpv4(inet_addr("1.1.1.1"));
    auto node1 = std::make_shared<Node>(nullptr, addr);
    nodes.AddNode(node1);
    addr.SetIpv4(inet_addr("1.1.1.2"));
    auto node2 = std::make_shared<Node>(nullptr, addr);
    nodes.AddNode(node2);
    addr.SetIpv4(inet_addr("1.1.1.3"));
    auto node3 = std::make_shared<Node>(nullptr, addr);
    nodes.AddNode(node3);
    
    NetAddr netmask;
    netmask.SetIpv4(inet_addr("255.255.255.0"));
    SubNet subnet(addr, netmask);
    
    ASSERT_NE(nodes.GetNode(node1->id()), nullptr);
    ASSERT_NE(nodes.GetNode(node2->id()), nullptr);
    ASSERT_NE(nodes.GetNode(node3->id()), nullptr);
    DisconnectNode(subnet);
    EXPECT_EQ(nodes.GetNode(node1->id()), nullptr);
    EXPECT_EQ(nodes.GetNode(node2->id()), nullptr);
    EXPECT_EQ(nodes.GetNode(node3->id()), nullptr);

    nodes.Clear();
}

} // namespace unit_test
} // namespace btclit
