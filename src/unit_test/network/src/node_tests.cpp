#include "node_tests.h"

#include "bandb.h"


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

TEST_F(NodesTest, ClearDisconnected)
{
    std::vector<std::shared_ptr<Node> > vec;
    auto node1 = nodes_.GetNode(id1_);
    auto node2 = nodes_.GetNode(id2_);
    
    node1->mutable_connection()->set_connection_state(NodeConnection::kDisconnected);
    node2->mutable_connection()->set_connection_state(NodeConnection::kDisconnected);
    nodes_.ClearDisconnected(&vec);
    EXPECT_EQ(nodes_.GetNode(id1_), nullptr);
    EXPECT_EQ(nodes_.GetNode(id2_), nullptr);
    ASSERT_EQ(vec.size(), 2);
    EXPECT_EQ(vec[0]->connection().addr(), addr1_);
    EXPECT_EQ(vec[1]->connection().addr(), addr2_);
}

TEST_F(NodesTest, DisconnectNode)
{
    NetAddr netmask;
    auto node1 = nodes_.GetNode(id1_);
    auto node2 = nodes_.GetNode(id2_);
    auto node3 = nodes_.GetNode(id3_);
    
    netmask.SetIpv4(inet_addr("255.255.255.0"));
    SubNet subnet(addr1_, netmask);
    nodes_.DisconnectNode(subnet);
    
    EXPECT_EQ(node1->connection().connection_state(), NodeConnection::kDisconnected);
    EXPECT_EQ(node2->connection().connection_state(), NodeConnection::kDisconnected);
    EXPECT_EQ(node3->connection().connection_state(), NodeConnection::kDisconnected);
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
    node->InactivityTimeoutCb(node);
    EXPECT_EQ(node->connection().connection_state(), NodeConnection::kDisconnected);
}

TEST_F(NodeTest, CheckBanned)
{
    Nodes& nodes = SingletonNodes::GetInstance();
    std::shared_ptr<Node> node = nodes.GetNode(id_);
    ASSERT_NE(node, nullptr);
    
    node->mutable_misbehavior()->set_should_ban(true);
    ASSERT_TRUE(node->CheckBanned());
    EXPECT_EQ(node->connection().connection_state(), NodeConnection::kDisconnected);
    EXPECT_TRUE(SingletonBanDb::GetInstance().IsBanned(addr_));

    SingletonBanDb::GetInstance().Clear();
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


} // namespace unit_test
} // namespace btclit
