#include "node_tests.h"

#include "bandb.h"


TEST_F(NodesTest, InitializeNode)
{
    std::shared_ptr<Node> result = nodes_.GetNode(addr3_);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->addr(), addr3_);
    EXPECT_TRUE(SingletonBlockSync::GetInstance().IsExist(id3_)); 
}

TEST_F(NodesTest, GetNodeByAddr)
{
    std::shared_ptr<Node> result = nodes_.GetNode(addr1_);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->addr(), addr1_);
}

TEST_F(NodesTest, GetNodeById)
{
    std::shared_ptr<Node> result = nodes_.GetNode(id2_);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->addr(), addr2_);
}

TEST_F(NodesTest, EraseNodeByPtr)
{
    std::shared_ptr<Node> node = nodes_.GetNode(id1_);
    ASSERT_NE(node, nullptr);
    nodes_.EraseNode(node);
    EXPECT_EQ(nodes_.GetNode(addr1_), nullptr);
    EXPECT_EQ(nodes_.GetNode(id1_), nullptr);
    EXPECT_NE(nodes_.GetNode(id2_), nullptr);
}

TEST_F(NodesTest, EraseNodeById)
{
    std::shared_ptr<Node> node = nodes_.GetNode(id2_);
    ASSERT_NE(node, nullptr);
    nodes_.EraseNode(id2_);
    EXPECT_EQ(nodes_.GetNode(addr2_), nullptr);
    EXPECT_EQ(nodes_.GetNode(id2_), nullptr);
    EXPECT_NE(nodes_.GetNode(id1_), nullptr);
}

TEST_F(NodesTest, CheckIncomingNonce)
{
    std::shared_ptr<Node> node1 = nodes_.GetNode(id1_);
    std::shared_ptr<Node> node2 = nodes_.GetNode(id2_);
    ASSERT_NE(node1, nullptr);
    ASSERT_NE(node2, nullptr);
    EXPECT_FALSE(nodes_.CheckIncomingNonce(node1->local_host_nonce()));
    EXPECT_TRUE(nodes_.CheckIncomingNonce(1234));
    EXPECT_TRUE(nodes_.CheckIncomingNonce(node2->local_host_nonce()));
    node1->set_conn_established(true);
    EXPECT_TRUE(nodes_.CheckIncomingNonce(node1->local_host_nonce()));
}

TEST_F(NodesTest, DisconnectNode)
{
    btclite::network::NetAddr addr1, addr2, addr3, netmask;
    
    addr1.SetIpv4(inet_addr("1.1.2.1"));
    nodes_.InitializeNode(nullptr, addr1);
    addr2.SetIpv4(inet_addr("1.1.2.2"));
    nodes_.InitializeNode(nullptr, addr2);    
    addr3.SetIpv4(inet_addr("1.1.2.3"));
    nodes_.InitializeNode(nullptr, addr3);
    
    netmask.SetIpv4(inet_addr("255.255.255.0"));
    SubNet subnet(addr1, netmask);
    nodes_.DisconnectNode(subnet);
    
    std::shared_ptr<Node> node = nodes_.GetNode(addr1_);
    EXPECT_FALSE(node->disconnected());
    
    node = nodes_.GetNode(addr2_);
    EXPECT_FALSE(node->disconnected());
    
    node = nodes_.GetNode(addr3_);
    EXPECT_FALSE(node->disconnected());
    
    node = nodes_.GetNode(addr1);
    EXPECT_TRUE(node->disconnected());
    
    node = nodes_.GetNode(addr2);
    EXPECT_TRUE(node->disconnected());
    
    node = nodes_.GetNode(addr3);
    EXPECT_TRUE(node->disconnected());
    
    SingletonBlockSync::GetInstance().Clear();
    SingletonNodes::GetInstance().Clear();
}

TEST_F(NodesTest, InboundCount)
{
    btclite::network::NetAddr addr;
    
    addr.SetIpv4(inet_addr("1.1.1.4"));
    nodes_.InitializeNode(nullptr, addr, false);
    addr.SetIpv4(inet_addr("1.1.1.5"));
    nodes_.InitializeNode(nullptr, addr, false);
    EXPECT_EQ(nodes_.CountInbound(), 2);
    EXPECT_EQ(nodes_.CountOutbound(), 3);
}

TEST_F(NodeTest, Disconnect)
{
    Nodes& nodes = SingletonNodes::GetInstance();
    std::shared_ptr<Node> node = nodes.GetNode(id_);
    ASSERT_NE(node, nullptr);
    node->set_disconnected(true);
    EXPECT_TRUE(node->disconnected());
    EXPECT_EQ(nodes.GetNode(id_), nullptr);
}

TEST_F(NodeTest, HandleInactiveTimeout)
{
    Nodes& nodes = SingletonNodes::GetInstance();
    std::shared_ptr<Node> node = nodes.GetNode(id_);
    ASSERT_NE(node, nullptr);
    node->InactivityTimeoutCb(node);
    EXPECT_TRUE(node->disconnected());
    EXPECT_EQ(nodes.GetNode(node->id()), nullptr);
}

TEST_F(NodeTest, CheckBanned)
{
    Nodes& nodes = SingletonNodes::GetInstance();
    BlockSync& block_sync = SingletonBlockSync::GetInstance();
    std::shared_ptr<Node> node = nodes.GetNode(id_);    
    block_sync.AddSyncState(id_, addr_, "");
    
    ASSERT_NE(node, nullptr);
    ASSERT_TRUE(block_sync.IsExist(id_));
    ASSERT_FALSE(node->CheckBanned());    
    block_sync.SetShouldBan(id_, true);
    ASSERT_TRUE(node->CheckBanned());
    EXPECT_TRUE(node->disconnected());
    EXPECT_TRUE(SingletonBanDb::GetInstance().IsBanned(addr_));
    
    block_sync.Clear();
    SingletonBanDb::GetInstance().Clear();
}

TEST_F(NodeTest, PushAddress)
{
    Nodes& nodes = SingletonNodes::GetInstance();
    std::shared_ptr<Node> node = nodes.GetNode(id_);
    node->mutable_known_addrs()->push_back(addr_.GetHash().GetLow64());
    EXPECT_FALSE(node->PushAddress(addr_));
    
    addr_.mutable_proto_addr()->set_port(8333);
    ASSERT_TRUE(node->PushAddress(addr_));
    EXPECT_EQ(node->addrs_to_send().front(), addr_);
}

TEST(DestructNodeTest, destructor)
{
    btclite::network::NetAddr addr;
    Nodes& nodes = SingletonNodes::GetInstance();
    BlockSync& block_sync = SingletonBlockSync::GetInstance();
    NodeId id;
    
    addr.SetIpv4(inet_addr("1.1.1.1"));
    {
        std::shared_ptr<Node> node = std::make_shared<Node>(nullptr, addr);
        nodes.AddNode(node);
        block_sync.AddSyncState(node->id(), node->addr(), node->host_name());
        ASSERT_TRUE(block_sync.IsExist(node->id()));
        node->InactivityTimeoutCb(node);
        id = node->id();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_FALSE(block_sync.IsExist(id));
    
    block_sync.Clear();
}
