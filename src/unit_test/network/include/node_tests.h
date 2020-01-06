#include <gtest/gtest.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "node.h"


namespace btclite {
namespace unit_test {

class NodesTest : public ::testing::Test {
protected:
    void SetUp() override 
    {
        addr1_.SetIpv4(inet_addr("1.1.1.1"));
        auto node1 = std::make_shared<network::Node>(nullptr, addr1_, false);
        id1_ = node1->id();
        nodes_.AddNode(node1);
        
        addr2_.SetIpv4(inet_addr("1.1.1.2"));
        auto node2 = std::make_shared<network::Node>(nullptr, addr2_);
        id2_ = node2->id();
        nodes_.AddNode(node2);
        
        addr3_.SetIpv4(inet_addr("1.1.1.3"));
        auto node3 = nodes_.InitializeNode(nullptr, addr3_);
        id3_ = node3->id();
    }
    
    void TearDown() override
    {
        network::SingletonBlockSync::GetInstance().Clear();
        network::SingletonNodes::GetInstance().Clear();
    }
    
    network::Nodes nodes_;
    network::NetAddr addr1_;
    network::NetAddr addr2_;
    network::NetAddr addr3_;
    network::NodeId id1_ = 0;
    network::NodeId id2_ = 0;
    network::NodeId id3_ = 0;
};

class NodeTest : public ::testing::Test {
protected:
    void SetUp() override {
        addr_.SetIpv4(inet_addr("1.1.1.1"));
        auto node = std::make_shared<network::Node>(nullptr, addr_);
        id_ = node->id();
        network::SingletonNodes::GetInstance().AddNode(node);
    }
    
    void TearDown() override
    {
        network::SingletonNodes::GetInstance().Clear();
    }
    
    network::NetAddr addr_;
    network::NodeId id_ = 0;
};

} // namespace unit_test
} // namespace btclit
