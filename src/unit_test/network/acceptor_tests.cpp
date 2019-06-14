#include <gtest/gtest.h>

#include "acceptor.h"


TEST(AcceptorTest, MethordInit)
{
    Network::Params params(BaseEnv::mainnet);
    Acceptor acceptor;
    
    ASSERT_TRUE(acceptor.Init(params));
    ASSERT_EQ(acceptor.listen_sockets().size(), 2);
    ASSERT_GT(acceptor.listen_sockets()[0].sock_fd(), 0);
    ASSERT_GT(acceptor.listen_sockets()[1].sock_fd(), 0);
    
    const struct sockaddr_in *p4 = reinterpret_cast<const struct sockaddr_in*>(&acceptor.listen_sockets()[0].sock_addr());
    const struct sockaddr_in6 *p6 = reinterpret_cast<const struct sockaddr_in6*>(&acceptor.listen_sockets()[1].sock_addr());
    EXPECT_EQ(p4->sin_family, AF_INET);
    EXPECT_EQ(p4->sin_addr.s_addr, htonl(INADDR_ANY));
    EXPECT_EQ(p4->sin_port, htons(params.default_port()));
    EXPECT_EQ(p6->sin6_family, AF_INET6);
    EXPECT_EQ(std::memcmp(p6->sin6_addr.s6_addr, in6addr_any.s6_addr, sizeof(in6addr_any)), 0);
    EXPECT_EQ(p6->sin6_port, htons(params.default_port()));
    EXPECT_EQ(p6->sin6_scope_id, 0);
    
    for (auto socket : acceptor.listen_sockets())
        socket.Close();
}

TEST(AcceptorTest, MethordBind)
{
    Network::Params params(BaseEnv::mainnet);
    Acceptor acceptor;
    
    ASSERT_TRUE(acceptor.Init(params));
    EXPECT_TRUE(acceptor.Bind());    
    for (auto socket : acceptor.listen_sockets())
        socket.Close();
    EXPECT_FALSE(acceptor.Bind());
}

TEST(AcceptorTest, MethordListen)
{
    Network::Params params(BaseEnv::mainnet);
    Acceptor acceptor;
    
    ASSERT_TRUE(acceptor.Init(params));
    ASSERT_TRUE(acceptor.Bind());
    EXPECT_TRUE(acceptor.Listen());
    for (auto socket : acceptor.listen_sockets())
        socket.Close();
    EXPECT_FALSE(acceptor.Listen());
}
