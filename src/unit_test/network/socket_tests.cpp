#include <gtest/gtest.h>

#include "socket.h"

TEST(BasicSocketTest, Constructor)
{
    struct sockaddr_in sock_addr4;
    struct sockaddr_in6 sock_addr6;
    
    memset(&sock_addr4, 0, sizeof(sock_addr4));
    memset(&sock_addr6, 0, sizeof(sock_addr6));
    
    sock_addr4.sin_family = AF_INET;
    sock_addr4.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_addr4.sin_port = htons(1234);
    BasicSocket socket(sock_addr4);
    ASSERT_EQ(std::memcmp(&socket.sock_addr(), &sock_addr4, sizeof(sock_addr4)), 0);
    
    sock_addr6.sin6_family = AF_INET6;
    std::memcpy(&sock_addr6.sin6_addr, &in6addr_any, sizeof(in6addr_any));
    sock_addr6.sin6_port = htons(1234);
    sock_addr6.sin6_scope_id = 1;
    BasicSocket socket2(sock_addr6);
    ASSERT_EQ(std::memcmp(&socket2.sock_addr(), &sock_addr6, sizeof(sock_addr6)), 0);
}

TEST(BasicSocketTest, MethordCreate)
{
    struct sockaddr_in sock_addr4;
    struct sockaddr_in6 sock_addr6;
    
    memset(&sock_addr4, 0, sizeof(sock_addr4));
    memset(&sock_addr6, 0, sizeof(sock_addr6));
    
    sock_addr4.sin_family = AF_INET;
    sock_addr4.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_addr4.sin_port = htons(1234);
    BasicSocket socket(sock_addr4);
    ASSERT_TRUE(socket.Create());
    EXPECT_GT(socket.sock_fd(), 0);
    EXPECT_TRUE(socket.Close());
    EXPECT_EQ(socket.sock_fd(), -1);
    
    sock_addr6.sin6_family = AF_INET6;
    std::memcpy(&sock_addr6.sin6_addr, &in6addr_any, sizeof(in6addr_any));
    sock_addr6.sin6_port = htons(1234);
    sock_addr6.sin6_scope_id = 1;
    BasicSocket socket2(sock_addr6);
    ASSERT_TRUE(socket2.Create());
    EXPECT_GT(socket2.sock_fd(), 0);
    EXPECT_TRUE(socket2.Close());
    EXPECT_EQ(socket2.sock_fd(), -1);
}

TEST(AcceptorTest, MethordInit)
{
    Network::Params params;
    Acceptor acceptor;
    
    params.Init(BaseEnv::mainnet);
    ASSERT_TRUE(acceptor.Init(params));
    ASSERT_EQ(acceptor.sockets().size(), 2);
    ASSERT_GT(acceptor.sockets()[0].sock_fd(), 0);
    ASSERT_GT(acceptor.sockets()[1].sock_fd(), 0);
    
    const struct sockaddr_in *p4 = reinterpret_cast<const struct sockaddr_in*>(&acceptor.sockets()[0].sock_addr());
    const struct sockaddr_in6 *p6 = reinterpret_cast<const struct sockaddr_in6*>(&acceptor.sockets()[1].sock_addr());
    EXPECT_EQ(p4->sin_family, AF_INET);
    EXPECT_EQ(p4->sin_addr.s_addr, htonl(INADDR_ANY));
    EXPECT_EQ(p4->sin_port, htons(params.default_port()));
    EXPECT_EQ(p6->sin6_family, AF_INET6);
    EXPECT_EQ(std::memcmp(p6->sin6_addr.s6_addr, in6addr_any.s6_addr, sizeof(in6addr_any)), 0);
    EXPECT_EQ(p6->sin6_port, htons(params.default_port()));
    EXPECT_EQ(p6->sin6_scope_id, 0);
    
    for (auto socket : acceptor.sockets())
        socket.Close();
}

TEST(AcceptorTest, MethordBind)
{
    Network::Params params;
    Acceptor acceptor;
    
    params.Init(BaseEnv::mainnet);
    ASSERT_TRUE(acceptor.Init(params));
    EXPECT_TRUE(acceptor.Bind());    
    for (auto socket : acceptor.sockets())
        socket.Close();
    EXPECT_FALSE(acceptor.Bind());
}

TEST(AcceptorTest, MethordListen)
{
    Network::Params params;
    Acceptor acceptor;
    
    params.Init(BaseEnv::mainnet);
    ASSERT_TRUE(acceptor.Init(params));
    ASSERT_TRUE(acceptor.Bind());
    EXPECT_TRUE(acceptor.Listen());
    for (auto socket : acceptor.sockets())
        socket.Close();
    EXPECT_FALSE(acceptor.Listen());
}
