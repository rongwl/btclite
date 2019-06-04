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
