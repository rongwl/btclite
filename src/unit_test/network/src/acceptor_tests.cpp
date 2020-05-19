#include <gtest/gtest.h>
#include <arpa/inet.h>

#include "acceptor.h"
#include "banlist.h"


namespace btclite {
namespace unit_test {

using namespace network;

TEST(AcceptorTest, Constructor)
{
    network::Params params(BtcNet::kTestNet, util::Args(), fs::path("/tmp/foo"));
    Acceptor acceptor(params);
    const struct sockaddr_in6& sock_addr = acceptor.sock_addr();
    ASSERT_EQ(sock_addr.sin6_family, AF_INET6);
    ASSERT_EQ(sock_addr.sin6_port, htons(18333));
    ASSERT_EQ(std::memcmp(sock_addr.sin6_addr.s6_addr, in6addr_any.s6_addr, 16), 0);
    ASSERT_EQ(sock_addr.sin6_scope_id, 0);
}

TEST(AcceptorTest, MethordAcceptConnCb)
{
    using namespace std::placeholders;
    
    network::Params params(BtcNet::kTestNet, util::Args(), fs::path("/tmp/foo"));
    Acceptor acceptor(params);
    struct event_base *base;
    struct evconnlistener *listener;
    struct bufferevent *bev;
    Socket::Fd fd;
    
    evthread_use_pthreads();
    base = event_base_new();
    ASSERT_NE(base, nullptr);
    listener = evconnlistener_new_bind(base, NULL, NULL, LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE,
                                       SOMAXCONN, (const struct sockaddr*)&acceptor.sock_addr(),
                                       sizeof(acceptor.sock_addr()));
    ASSERT_NE(listener, nullptr);
    
    fd = evconnlistener_get_fd(listener);
    ASSERT_GT(fd, 0);
    
    struct sockaddr_in6 client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin6_family = AF_INET6;
    client_addr.sin6_port = htons(18333);
    int count = 0;
    for (int i = 1; i < kMaxInboundConnections; i++) {
        std::string str_addr = "::ffff:1.2.3." + std::to_string(i);
        inet_pton(AF_INET6, str_addr.c_str(), client_addr.sin6_addr.s6_addr);
        NetAddr addr(client_addr);
        acceptor.AcceptConnCb(listener, fd, (struct sockaddr*)&client_addr, sizeof(client_addr), nullptr);
        EXPECT_EQ(Acceptor::Inbounds().Size(), i);
        count = i;
        
        std::shared_ptr<Node> pnode = Acceptor::Inbounds().GetNode(addr);
        ASSERT_NE(pnode, nullptr);
        EXPECT_EQ(pnode->connection().addr(), addr);
    }
    
    inet_pton(AF_INET6, "::ffff:1.2.3.250", client_addr.sin6_addr.s6_addr);
    NetAddr addr(client_addr);
    SingletonBanList::GetInstance().Add(addr, BanList::BanReason::kNodeMisbehaving);
    acceptor.AcceptConnCb(listener, fd, (struct sockaddr*)&client_addr, sizeof(client_addr), nullptr);
    EXPECT_EQ(Acceptor::Inbounds().Size(), count);  
    
    SingletonBanList::GetInstance().Erase(addr);
    acceptor.AcceptConnCb(listener, fd, (struct sockaddr*)&client_addr, sizeof(client_addr), nullptr);
    EXPECT_EQ(Acceptor::Inbounds().Size(), ++count);
    std::shared_ptr<Node> pnode = Acceptor::Inbounds().GetNode(addr);
    ASSERT_NE(pnode, nullptr);
    EXPECT_EQ(pnode->connection().addr(), addr);
    
    inet_pton(AF_INET6, "::ffff:1.2.3.251", client_addr.sin6_addr.s6_addr);
    acceptor.AcceptConnCb(listener, fd, (struct sockaddr*)&client_addr, sizeof(client_addr), nullptr);
    EXPECT_EQ(Acceptor::Inbounds().Size(), count);
    
    Acceptor::Inbounds().Clear();
    evconnlistener_free(listener);
    event_base_free(base);
}

} // namespace unit_test
} // namespace btclit
