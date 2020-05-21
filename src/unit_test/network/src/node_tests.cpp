#include "node_tests.h"

#include "banlist.h"
#include "block_sync.h"


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

TEST_F(NodesTest, CountSyncStarted)
{
    auto node1 = nodes_.GetNode(id1_);
    auto node2 = nodes_.GetNode(id2_);
    
    EXPECT_EQ(node1->block_sync_state().NSyncStarted(), 0);
    EXPECT_EQ(node2->block_sync_state().NSyncStarted(), 0);
    node1->mutable_block_sync_state()->set_sync_started(true);
    node2->mutable_block_sync_state()->set_sync_started(true);
    EXPECT_EQ(node1->block_sync_state().NSyncStarted(), 2);
    EXPECT_EQ(node2->block_sync_state().NSyncStarted(), 2);
}

TEST_F(NodesTest, CountPreferredDownload)
{
    auto node1 = nodes_.GetNode(id1_);
    auto node2 = nodes_.GetNode(id2_);
    
    EXPECT_EQ(node1->NPreferedDownload(), 0);
    EXPECT_EQ(node2->NPreferedDownload(), 0);
    node1->set_services(kNodeNetwork);
    EXPECT_EQ(node1->NPreferedDownload(), 1);
    EXPECT_EQ(node2->NPreferedDownload(), 1);
}

TEST_F(NodesTest, CountValidatedDownload)
{
    auto node1 = nodes_.GetNode(id1_);
    auto node2 = nodes_.GetNode(id2_);
    
    EXPECT_EQ(node1->blocks_in_flight().NValidatedDownload(), 0);
    EXPECT_EQ(node2->blocks_in_flight().NValidatedDownload(), 0);
    node1->mutable_blocks_in_flight()->set_n_valid_headers(1);
    node2->mutable_blocks_in_flight()->set_n_valid_headers(1);
    EXPECT_EQ(node1->blocks_in_flight().NValidatedDownload(), 2);
    EXPECT_EQ(node2->blocks_in_flight().NValidatedDownload(), 2);
}

TEST_F(NodeTest, Misbehaving)
{
    auto node = nodes_.GetNode(id_);
    
    node->mutable_misbehavior()->Misbehaving(id_, kDefaultBanscoreThreshold - 1);
    ASSERT_FALSE(node->misbehavior().should_ban());
    node->mutable_misbehavior()->Misbehaving(id_, 1);
    EXPECT_TRUE(node->misbehavior().should_ban());
}

TEST_F(NodeTest, HandleInactiveTimeout)
{
    std::shared_ptr<Node> node = nodes_.GetNode(id_);
    ASSERT_NE(node, nullptr);
    node->InactivityTimeoutCb();
    EXPECT_TRUE(node->connection().IsDisconnected());
}

TEST_F(NodeTest, CheckBanned)
{
    std::shared_ptr<Node> node = nodes_.GetNode(id_);
    BanList banlist;
    
    ASSERT_NE(node, nullptr);
    
    node->mutable_misbehavior()->set_should_ban(true);
    ASSERT_TRUE(node->CheckBanned(&banlist));
    EXPECT_TRUE(node->connection().IsDisconnected());
    EXPECT_TRUE(banlist.IsBanned(addr_));
}

TEST_F(NodeTest, PushAddrToSend)
{
    std::shared_ptr<Node> node = nodes_.GetNode(id_);
    node->mutable_flooding_addrs()->AddKnownAddr(addr_);
    EXPECT_FALSE(node->mutable_flooding_addrs()->PushAddrToSend(addr_));
    
    addr_.set_port(8333);
    ASSERT_TRUE(node->mutable_flooding_addrs()->PushAddrToSend(addr_));
    EXPECT_EQ(node->flooding_addrs().addrs_to_send().front(), addr_);
}

TEST(DisconnectedNode, DisconnectedNodeCb)
{
    NetAddr addr;
    Nodes nodes;
    Peers peers;
    BlocksInFlight1 blocks_in_flight;
    Orphans orphans;
    
    addr.SetIpv4(inet_addr("1.1.1.1"));
    auto node = std::make_shared<Node>(nullptr, addr);
    node->mutable_connection()->RegisterSetStateCb(NodeConnection::kDisconnected,
                std::bind(DisconnectNodeCb, node, &nodes, &peers, &blocks_in_flight, &orphans));
    nodes.AddNode(node);
    
    ASSERT_NE(nodes.GetNode(node->id()), nullptr);
    node->mutable_connection()->set_connection_state(NodeConnection::kDisconnected);
    EXPECT_EQ(nodes.GetNode(node->id()), nullptr);
    
    nodes.Clear();
}
#if 0
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
#endif
} // namespace unit_test
} // namespace btclit
