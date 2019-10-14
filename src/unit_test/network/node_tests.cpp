#include <gtest/gtest.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "node.h"


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
    EXPECT_NE(SingletonBlockSync::GetInstance().GetSyncState(node->id()), nullptr);
}

TEST(NodesTest, ModifyNode)
{
    Nodes nodes;
    btclite::network::NetAddr addr;
    addr.SetIpv4(inet_addr("1.1.1.1"));
    std::shared_ptr<Node> node = std::make_shared<Node>(nullptr, addr);
    
    nodes.AddNode(node);
    std::shared_ptr<Node> result = nodes.GetNode(node->id());
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->addr(), addr);     
    
    result = nodes.GetNode(addr);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->addr(), addr);

    nodes.EraseNode(node);
    result = nodes.GetNode(addr);
    EXPECT_EQ(result, nullptr);
    
    addr.SetIpv4(inet_addr("1.1.1.2"));
    node = std::make_shared<Node>(nullptr, addr);
    nodes.AddNode(node);
    result = nodes.GetNode(addr);
    ASSERT_NE(result, nullptr);
    nodes.EraseNode(node);
    result = nodes.GetNode(node->id());
    EXPECT_EQ(result, nullptr);
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


TEST(NodeTest, HandleInactiveTimeout)
{
    btclite::network::NetAddr addr;
    Nodes& nodes = SingletonNodes::GetInstance();
    
    addr.SetIpv4(inet_addr("1.1.1.1"));
    std::shared_ptr<Node> node = std::make_shared<Node>(nullptr, addr);
    nodes.AddNode(node);
    ASSERT_NE(nodes.GetNode(node->id()), nullptr);
    node->InactivityTimeoutCb(node);
    EXPECT_TRUE(node->disconnected());
    EXPECT_EQ(nodes.GetNode(node->id()), nullptr);
    
    nodes.Clear();
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
        ASSERT_NE(block_sync.GetSyncState(node->id()), nullptr);
        node->InactivityTimeoutCb(node);
        id = node->id();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_EQ(block_sync.GetSyncState(id), nullptr);
    
    nodes.Clear();
}
