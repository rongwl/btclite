#include <gtest/gtest.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "socket.h"

TEST(BasicSocketTest, Constructor)
{    
    BasicSocket socket1;
    ASSERT_EQ(socket1.sock_fd(), 0);
    
    BasicSocket socket2(3);
    ASSERT_EQ(socket2.sock_fd(), 3);
}

TEST(BasicSocketTest, MethordCreate)
{
    BasicSocket socket;
    ASSERT_TRUE(socket.Create());
    EXPECT_GT(socket.sock_fd(), 0);
    EXPECT_TRUE(socket.Close());
    EXPECT_EQ(socket.sock_fd(), -1);
}

TEST(BasicSocketTest, MethordGetBindAddr)
{
    Socket sock_fd;
    btclite::NetAddr addr;
    struct sockaddr_in sock_addr1, sock_addr2;
    struct sockaddr_in6 sock_addr3, sock_addr4;
    socklen_t len;
    
    std::memset(&sock_addr1, 0, sizeof(sock_addr1));
    sock_addr1.sin_family = AF_INET;
    sock_addr1.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_addr1.sin_port = htons(8333);
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GT(sock_fd, 0);
    EXPECT_EQ(bind(sock_fd, (const struct sockaddr*)&sock_addr1, sizeof(sock_addr1)), 0);
    BasicSocket socket1(sock_fd);
    EXPECT_TRUE(socket1.GetBindAddr(&addr));
    std::memset(&sock_addr2, 0, sizeof(sock_addr2));
    len = sizeof(sock_addr2);
    EXPECT_TRUE(addr.ToSockAddr((struct sockaddr*)&sock_addr2, &len));
    EXPECT_EQ(len, sizeof(sockaddr_in));
    EXPECT_EQ(std::memcmp(&sock_addr1, &sock_addr2, len), 0);
    socket1.Close();

    std::memset(&sock_addr3, 0, sizeof(sock_addr3));
    sock_addr3.sin6_family = AF_INET6;
    std::memcpy(&sock_addr3.sin6_addr, &in6addr_any, sizeof(in6addr_any));
    sock_addr3.sin6_port = htons(8333);
    sock_addr3.sin6_scope_id = 0;
    sock_fd = socket(AF_INET6, SOCK_STREAM, 0);
    ASSERT_GT(sock_fd, 0);
    EXPECT_EQ(bind(sock_fd, (const struct sockaddr*)&sock_addr3, sizeof(sock_addr3)), 0);
    BasicSocket socket2(sock_fd);
    EXPECT_TRUE(socket2.GetBindAddr(&addr));
    std::memset(&sock_addr4, 0, sizeof(sock_addr4));
    len = sizeof(sock_addr4);
    EXPECT_TRUE(addr.ToSockAddr((struct sockaddr*)&sock_addr4, &len));
    EXPECT_EQ(len, sizeof(sockaddr_in6));
    EXPECT_EQ(std::memcmp(&sock_addr3, &sock_addr4, sizeof(sockaddr_in6)), 0);
    socket2.Close();
}
