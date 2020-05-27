#include "acceptor_tests.h"


namespace btclite {
namespace unit_test {

using namespace network;

TEST_F(AcceptorTest, Constructor)
{
    const struct sockaddr_in6& sock_addr = acceptor_.sock_addr();
    ASSERT_EQ(sock_addr.sin6_family, AF_INET6);
    ASSERT_EQ(sock_addr.sin6_port, htons(18333));
    ASSERT_EQ(std::memcmp(sock_addr.sin6_addr.s6_addr, in6addr_any.s6_addr, 16), 0);
    ASSERT_EQ(sock_addr.sin6_scope_id, 0);
}

TEST_F(AcceptorTest, MethordAcceptConnCb)
{    
    evthread_use_pthreads();
    base_ = event_base_new();
    ASSERT_NE(base_, nullptr);
    listener_ = evconnlistener_new_bind(base_, NULL, NULL, LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE,
                                       SOMAXCONN, (const struct sockaddr*)&acceptor_.sock_addr(),
                                       sizeof(acceptor_.sock_addr()));
    ASSERT_NE(listener_, nullptr);
    
    fd_ = evconnlistener_get_fd(listener_);
    ASSERT_GT(fd_, 0);
    
    int count = 0;
    for (int i = 1; i < kMaxInboundConnections; i++) {
        std::string str_addr = "::ffff:1.2.3." + std::to_string(i);
        inet_pton(AF_INET6, str_addr.c_str(), client_addr_.sin6_addr.s6_addr);
        NetAddr addr(client_addr_);
        acceptor_.AcceptConnCb(listener_, fd_, (struct sockaddr*)&client_addr_, sizeof(client_addr_), nullptr);
        EXPECT_EQ(Acceptor::Inbounds().Size(), i);
        count = i;
        
        std::shared_ptr<Node> pnode = Acceptor::Inbounds().GetNode(addr);
        ASSERT_NE(pnode, nullptr);
        EXPECT_EQ(pnode->connection().addr(), addr);
    }

    inet_pton(AF_INET6, "::ffff:1.2.3.250", client_addr_.sin6_addr.s6_addr);
    NetAddr addr(client_addr_);
    ban_list_.Add(addr, BanList::BanReason::kNodeMisbehaving);
    acceptor_.AcceptConnCb(listener_, fd_, (struct sockaddr*)&client_addr_, sizeof(client_addr_), &ctx_);
    EXPECT_EQ(Acceptor::Inbounds().Size(), count);  
    
    ban_list_.Erase(addr);
    acceptor_.AcceptConnCb(listener_, fd_, (struct sockaddr*)&client_addr_, sizeof(client_addr_), &ctx_);
    EXPECT_EQ(Acceptor::Inbounds().Size(), ++count);
    std::shared_ptr<Node> pnode = Acceptor::Inbounds().GetNode(addr);
    ASSERT_NE(pnode, nullptr);
    EXPECT_EQ(pnode->connection().addr(), addr);
    
    inet_pton(AF_INET6, "::ffff:1.2.3.251", client_addr_.sin6_addr.s6_addr);
    acceptor_.AcceptConnCb(listener_, fd_, (struct sockaddr*)&client_addr_, sizeof(client_addr_), nullptr);
    EXPECT_EQ(Acceptor::Inbounds().Size(), count);
  
    Acceptor::Inbounds().Clear();
    evconnlistener_free(listener_);
    event_base_free(base_);
}

} // namespace unit_test
} // namespace btclit
