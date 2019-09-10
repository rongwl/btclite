#include <gtest/gtest.h>
#include <arpa/inet.h>

#include "acceptor.h"
#include "bandb.h"


TEST(AcceptorTest, Constructor)
{
    Acceptor acceptor;
    const struct sockaddr_in6& sock_addr = acceptor.sock_addr();
    ASSERT_EQ(sock_addr.sin6_family, AF_INET6);
    ASSERT_EQ(sock_addr.sin6_port, htons(Network::SingletonParams::GetInstance().default_port()));
    ASSERT_EQ(std::memcmp(sock_addr.sin6_addr.s6_addr, in6addr_any.s6_addr, 16), 0);
    ASSERT_EQ(sock_addr.sin6_scope_id, 0);
}

TEST(AcceptorTest, MethordAcceptConnCb)
{
    Acceptor acceptor;
    struct event_base *base;
    struct evconnlistener *listener;
    struct bufferevent *bev;
    Socket::Fd fd;
    
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
    client_addr.sin6_port = htons(8333);
    int count = 0;
    for (int i = 1; i < max_inbound_connections; i++) {
        std::string str_addr = "::ffff:1.2.3." + std::to_string(i);
        inet_pton(AF_INET6, str_addr.c_str(), client_addr.sin6_addr.s6_addr);
        btclite::NetAddr addr(client_addr);
        acceptor.AcceptConnCb(listener, fd, (struct sockaddr*)&client_addr, sizeof(client_addr), nullptr);
        ASSERT_EQ(SingletonNodes::GetInstance().CountInbound(), i);
        count = i;
        
        std::shared_ptr<Node> pnode = SingletonNodes::GetInstance().GetNode(addr);
        ASSERT_NE(pnode, nullptr);
        ASSERT_EQ(pnode->addr(), addr);
        
        const BlockSyncState* const pstate = SingletonBlockSync::GetInstance().GetSyncState(pnode->id());
        ASSERT_NE(pstate, nullptr);
        ASSERT_EQ(pstate->node_addr(), addr);
    }
    
    inet_pton(AF_INET6, "::ffff:1.2.3.250", client_addr.sin6_addr.s6_addr);
    btclite::NetAddr addr(client_addr);
    SingletonBanDb::GetInstance().Add(addr, BanDb::NodeMisbehaving);
    acceptor.AcceptConnCb(listener, fd, (struct sockaddr*)&client_addr, sizeof(client_addr), nullptr);
    EXPECT_EQ(SingletonNodes::GetInstance().CountInbound(), count);  
    
    SingletonBanDb::GetInstance().Erase(addr, false);
    acceptor.AcceptConnCb(listener, fd, (struct sockaddr*)&client_addr, sizeof(client_addr), nullptr);
    EXPECT_EQ(SingletonNodes::GetInstance().CountInbound(), ++count);
    std::shared_ptr<Node> pnode = SingletonNodes::GetInstance().GetNode(addr);
    ASSERT_NE(pnode, nullptr);
    EXPECT_EQ(pnode->addr(), addr);
    const BlockSyncState* const pstate = SingletonBlockSync::GetInstance().GetSyncState(pnode->id());
    ASSERT_NE(pstate, nullptr);
    ASSERT_EQ(pstate->node_addr(), addr);
    
    inet_pton(AF_INET6, "::ffff:1.2.3.251", client_addr.sin6_addr.s6_addr);
    acceptor.AcceptConnCb(listener, fd, (struct sockaddr*)&client_addr, sizeof(client_addr), nullptr);
    EXPECT_EQ(SingletonNodes::GetInstance().CountInbound(), count);
    
    evconnlistener_free(listener);
    event_base_free(base);
}
