#include <gtest/gtest.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "socket.h"

TEST(BasicSocketTest, Constructor)
{
    struct sockaddr_in sock_addr4;
    struct sockaddr_in6 sock_addr6;
    
    memset(&sock_addr4, 0, sizeof(sock_addr4));
    memset(&sock_addr6, 0, sizeof(sock_addr6));
    
    sock_addr4.sin_family = AF_INET;
    sock_addr4.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_addr4.sin_port = htons(8333);
    BasicSocket socket(AF_INET);
    ASSERT_EQ(std::memcmp(&socket.sock_addr(), &sock_addr4, sizeof(sock_addr4)), 0);
    
    sock_addr6.sin6_family = AF_INET6;
    std::memcpy(&sock_addr6.sin6_addr, &in6addr_any, sizeof(in6addr_any));
    sock_addr6.sin6_port = htons(8333);
    sock_addr6.sin6_scope_id = 0;
    BasicSocket socket2(AF_INET6);
    ASSERT_EQ(std::memcmp(&socket2.sock_addr(), &sock_addr6, sizeof(sock_addr6)), 0);
}

TEST(BasicSocketTest, MethordCreate)
{
    struct sockaddr_in sock_addr4;
    struct sockaddr_in6 sock_addr6;
    
    memset(&sock_addr4, 0, sizeof(sock_addr4));
    memset(&sock_addr6, 0, sizeof(sock_addr6));
    
    BasicSocket socket(AF_INET);
    ASSERT_TRUE(socket.Create());
    EXPECT_GT(socket.sock_fd(), 0);
    EXPECT_TRUE(socket.Close());
    EXPECT_EQ(socket.sock_fd(), -1);
    
    BasicSocket socket2(AF_INET6);
    ASSERT_TRUE(socket2.Create());
    EXPECT_GT(socket2.sock_fd(), 0);
    EXPECT_TRUE(socket2.Close());
    EXPECT_EQ(socket2.sock_fd(), -1);
}

TEST(BasicSocketTest, MethordGetBindAddr)
{
    Socket sock_fd;
    btclite::NetAddr addr;
    
    BasicSocket socket4(AF_INET);
    ASSERT_TRUE(socket4.Create());
    ASSERT_EQ(bind(socket4.sock_fd(), reinterpret_cast<const struct sockaddr*>(&socket4.sock_addr()), sizeof(socket4.sock_addr())), 0);
    BasicSocket::GetBindAddr(socket4.sock_fd(), &addr);
    EXPECT_EQ(addr, btclite::NetAddr(socket4.sock_addr()));
    socket4.Close();
    
    BasicSocket socket6(AF_INET6);
    ASSERT_TRUE(socket6.Create());
    ASSERT_EQ(bind(socket6.sock_fd(), reinterpret_cast<const struct sockaddr*>(&socket6.sock_addr()), sizeof(socket6.sock_addr())), 0);
    addr.Clear();
    BasicSocket::GetBindAddr(socket6.sock_fd(), &addr);
    EXPECT_EQ(addr, btclite::NetAddr(socket6.sock_addr()));
    socket6.Close();
}
