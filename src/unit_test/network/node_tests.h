#include <gtest/gtest.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "node.h"


class FixtureNodesTest : public ::testing::Test {
protected:
    void SetUp() override {
        addr1_.SetIpv4(inet_addr("1.1.1.1"));
        std::shared_ptr<Node> node1 = std::make_shared<Node>(nullptr, addr1_);
        id1_ = node1->id();
        nodes_.AddNode(node1);
        
        addr2_.SetIpv4(inet_addr("1.1.1.2"));
        std::shared_ptr<Node> node2 = std::make_shared<Node>(nullptr, addr2_);
        id2_ = node2->id();
        nodes_.AddNode(node2);
    }
    
    Nodes nodes_;
    btclite::network::NetAddr addr1_;
    btclite::network::NetAddr addr2_;
    NodeId id1_;
    NodeId id2_;
};

class FixtureNodeTest : public ::testing::Test {
protected:
    void SetUp() override {
        addr_.SetIpv4(inet_addr("1.1.1.1"));
        std::shared_ptr<Node> node = std::make_shared<Node>(nullptr, addr_);
        id_ = node->id();
        SingletonNodes::GetInstance().AddNode(node);
    }
    
    btclite::network::NetAddr addr_;
    NodeId id_;
};
