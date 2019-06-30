#include <gtest/gtest.h>
#include <arpa/inet.h>

#include "acceptor.h"
#include "socket_tests.h"


TEST(AcceptorTest, Constructor)
{
    Network::Params& params = Network::SingletonParams::GetInstance(BaseEnv::mainnet);
    Acceptor acceptor;
    ASSERT_GT(SingletonListenSocket::GetInstance().sock_fd(), 0);
}

TEST(AcceptorTest, MethodListenAndBind)
{    
    Network::Params& params = Network::SingletonParams::GetInstance(BaseEnv::mainnet);
    struct sockaddr_in6 sockaddr;
    socklen_t len = sizeof(sockaddr);
    Acceptor acceptor;
    
    ASSERT_TRUE(acceptor.BindAndListen());
    Socket& socket = SingletonListenSocket::GetInstance();    
    EXPECT_EQ(getsockname(socket.sock_fd(), (struct sockaddr*)&sockaddr, &len), 0);
    EXPECT_EQ(sockaddr.sin6_family, AF_INET6);
    EXPECT_EQ(std::memcmp(sockaddr.sin6_addr.s6_addr, in6addr_any.s6_addr, sizeof(in6addr_any)), 0);
    EXPECT_EQ(sockaddr.sin6_port, htons(params.default_port()));
    EXPECT_EQ(sockaddr.sin6_scope_id, 0);
    socket.Close();
    EXPECT_FALSE(acceptor.BindAndListen());
}

TEST(AcceptorTest, MethodAccept)
{
    MockSocket socket;
    Acceptor acceptor(socket);
    struct sockaddr_in6 sock_addr;
    struct sockaddr_storage *paddr = reinterpret_cast<struct sockaddr_storage*>(&sock_addr);

    std::memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin6_family = AF_INET6;
    sock_addr.sin6_port = htons(8333);
    sock_addr.sin6_scope_id = 0;    
    EXPECT_CALL(socket, Close()).Times(1).WillOnce(testing::Return(true));
    for (int i = 1; i < 10; i++) {
        std::string addr = "::ffff:1.2.3." + std::to_string(i);
        inet_pton(AF_INET6, addr.c_str(), sock_addr.sin6_addr.s6_addr);    
        EXPECT_CALL(socket, Accept(testing::_, testing::_)).Times(1).\
        WillOnce(testing::DoAll(testing::SetArgPointee<0>(*paddr), testing::Return(i+1)));
        ASSERT_TRUE(acceptor.Accept());
    }
    
}
