#include "node_tests.h"

#include "bandb.h"


TEST(NodesTest, InitializeNode)
{
    Nodes nodes;
    btclite::network::NetAddr addr;
    
    addr.SetIpv4(inet_addr("1.1.1.1"));
    std::shared_ptr<Node> node = nodes.InitializeNode(nullptr, addr);
    ASSERT_NE(node, nullptr);
    std::shared_ptr<Node> result = nodes.GetNode(addr);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->addr(), addr);
    EXPECT_TRUE(SingletonBlockSync::GetInstance().IsExist(node->id()));
}

TEST_F(FixtureNodesTest, GetNodeByAddr)
{
    std::shared_ptr<Node> result = nodes_.GetNode(addr1_);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->addr(), addr1_);
}

TEST_F(FixtureNodesTest, GetNodeById)
{
    std::shared_ptr<Node> result = nodes_.GetNode(id2_);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->addr(), addr2_);
}

TEST_F(FixtureNodesTest, EraseNodeByPtr)
{
    std::shared_ptr<Node> node = nodes_.GetNode(id1_);
    ASSERT_NE(node, nullptr);
    nodes_.EraseNode(node);
    EXPECT_EQ(nodes_.GetNode(addr1_), nullptr);
    EXPECT_EQ(nodes_.GetNode(id1_), nullptr);
    EXPECT_NE(nodes_.GetNode(id2_), nullptr);
}

TEST_F(FixtureNodesTest, EraseNodeById)
{
    std::shared_ptr<Node> node = nodes_.GetNode(id2_);
    ASSERT_NE(node, nullptr);
    nodes_.EraseNode(id2_);
    EXPECT_EQ(nodes_.GetNode(addr2_), nullptr);
    EXPECT_EQ(nodes_.GetNode(id2_), nullptr);
    EXPECT_NE(nodes_.GetNode(id1_), nullptr);
}

TEST(NodesTest, DisconnectNode)
{
    Nodes nodes;
    btclite::network::NetAddr addr, netmask;
    
    addr.SetIpv4(inet_addr("1.1.1.1"));
    nodes.InitializeNode(nullptr, addr);
    addr.SetIpv4(inet_addr("1.1.2.1"));
    nodes.InitializeNode(nullptr, addr);
    addr.SetIpv4(inet_addr("1.1.2.2"));
    nodes.InitializeNode(nullptr, addr);    
    addr.SetIpv4(inet_addr("1.1.2.3"));
    nodes.InitializeNode(nullptr, addr);
    
    netmask.SetIpv4(inet_addr("255.255.255.0"));
    SubNet subnet(addr, netmask);
    nodes.DisconnectNode(subnet);
    
    addr.SetIpv4(inet_addr("1.1.1.1"));
    std::shared_ptr<Node> node = nodes.GetNode(addr);
    EXPECT_FALSE(node->disconnected());
    
    addr.SetIpv4(inet_addr("1.1.2.1"));
    node = nodes.GetNode(addr);
    EXPECT_TRUE(node->disconnected());
    
    addr.SetIpv4(inet_addr("1.1.2.2"));
    node = nodes.GetNode(addr);
    EXPECT_TRUE(node->disconnected());
    
    addr.SetIpv4(inet_addr("1.1.2.3"));
    node = nodes.GetNode(addr);
    EXPECT_TRUE(node->disconnected());
}

TEST(NodesTest, InboundCount)
{
    Nodes nodes;
    btclite::network::NetAddr addr;
    
    addr.SetIpv4(inet_addr("1.1.1.1"));
    nodes.InitializeNode(nullptr, addr);
    addr.SetIpv4(inet_addr("1.1.1.2"));
    nodes.InitializeNode(nullptr, addr);
    addr.SetIpv4(inet_addr("1.1.1.3"));
    nodes.InitializeNode(nullptr, addr);
    addr.SetIpv4(inet_addr("1.1.1.4"));
    nodes.InitializeNode(nullptr, addr, false);
    addr.SetIpv4(inet_addr("1.1.1.5"));
    nodes.InitializeNode(nullptr, addr, false);
    EXPECT_EQ(nodes.CountInbound(), 3);
    EXPECT_EQ(nodes.CountOutbound(), 2);
}

TEST_F(FixtureNodeTest, Disconnect)
{
    Nodes& nodes = SingletonNodes::GetInstance();
    std::shared_ptr<Node> node = nodes.GetNode(id_);
    ASSERT_NE(node, nullptr);
    node->set_disconnected(true);
    EXPECT_TRUE(node->disconnected());
    EXPECT_EQ(nodes.GetNode(id_), nullptr);
    
    nodes.Clear();
}

TEST_F(FixtureNodeTest, HandleInactiveTimeout)
{
    Nodes& nodes = SingletonNodes::GetInstance();
    std::shared_ptr<Node> node = nodes.GetNode(id_);
    ASSERT_NE(node, nullptr);
    node->InactivityTimeoutCb(node);
    EXPECT_TRUE(node->disconnected());
    EXPECT_EQ(nodes.GetNode(node->id()), nullptr);
    
    nodes.Clear();
}

TEST_F(FixtureNodeTest, CheckBanned)
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
    
    nodes.Clear();
    SingletonBanDb::GetInstance().Clear();
}

TEST(NodeTest, destructor)
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
    
    nodes.Clear();
}
