#include <gtest/gtest.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "netbase.h"


TEST(NetBaseTest, LookupHost)
{
    btclite::NetAddr addr;
    uint8_t buf[sizeof(struct in6_addr)];
    uint8_t out[btclite::NetAddr::ip_byte_size];
    
    LookupHost("127.0.0.1", &addr, false);
    ASSERT_TRUE(addr.IsIpv4());
    EXPECT_EQ(addr.GetIpv4(), inet_addr("127.0.0.1"));
    
    addr.Clear();
    LookupHost("::FFFF:192.168.1.1", &addr, false);
    ASSERT_TRUE(addr.IsIpv4());
    EXPECT_EQ(addr.GetIpv4(), inet_addr("192.168.1.1"));
    
    addr.Clear();
    memset(buf, 0, sizeof(buf));
    memset(out, 0, sizeof(out));
    inet_pton(AF_INET6, "::1", buf);
    LookupHost("::1", &addr, false);
    ASSERT_TRUE(addr.IsIpv6());
    addr.GetIpv6(out);
    EXPECT_EQ(std::memcmp(buf, out, sizeof(out)), 0);    
}

TEST(NetBaseTest, LookupSubNet)
{
    btclite::NetAddr addr;
    uint8_t buf[sizeof(struct in6_addr)];
    
    addr.SetIpv4(inet_addr("192.168.1.1"));
    SubNet subnet1(addr, 24), subnet2;
    LookupSubNet("192.168.1.1/24", &subnet2);
    ASSERT_TRUE(subnet2.IsValid());
    EXPECT_EQ(subnet1, subnet2);
    
    inet_pton(AF_INET6, "1:2:3:5:5:6:7:8", buf);
    addr.SetIpv6(buf);
    SubNet subnet3(addr, 64);
    subnet2.Clear();
    LookupSubNet("1:2:3:5:5:6:7:8/64", &subnet2);
    ASSERT_TRUE(subnet3.IsValid());
    EXPECT_EQ(subnet3, subnet2);
}
