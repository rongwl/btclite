#include <gtest/gtest.h>

#include "acceptor.h"


TEST(AcceptorTest, Constructor)
{
    Network::Params& params = Network::SingletonParams::GetInstance(BaseEnv::mainnet);
    Acceptor acceptor;
    BasicSocket socket = acceptor.listen_socket();
    ASSERT_GT(socket.sock_fd(), 0);
    socket.Close();
}

TEST(AcceptorTest, MethordBind)
{
    Network::Params& params = Network::SingletonParams::GetInstance(BaseEnv::mainnet);
    struct sockaddr_in6 sockaddr;
    socklen_t len = sizeof(sockaddr);
    Acceptor acceptor;
    
    ASSERT_TRUE(acceptor.Bind());
    BasicSocket socket = acceptor.listen_socket();
    
    EXPECT_EQ(getsockname(socket.sock_fd(), (struct sockaddr*)&sockaddr, &len), 0);
    EXPECT_EQ(sockaddr.sin6_family, AF_INET6);
    EXPECT_EQ(std::memcmp(sockaddr.sin6_addr.s6_addr, in6addr_any.s6_addr, sizeof(in6addr_any)), 0);
    EXPECT_EQ(sockaddr.sin6_port, htons(params.default_port()));
    EXPECT_EQ(sockaddr.sin6_scope_id, 0);
    socket.Close();
    EXPECT_FALSE(acceptor.Bind());
}

TEST(AcceptorTest, MethordListen)
{
    Network::SingletonParams::GetInstance(BaseEnv::mainnet);
    Acceptor acceptor;
    
    ASSERT_TRUE(acceptor.Bind());
    EXPECT_TRUE(acceptor.Listen());
    BasicSocket socket = acceptor.listen_socket();
    socket.Close();
    EXPECT_FALSE(acceptor.Listen());
}
