#include <gtest/gtest.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

#include "network_address.h"
#include "constants.h"

TEST(NetAddrTest, Constructor)
{
    btclite::NetAddr addr1;
    uint8_t ip_none[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    EXPECT_EQ(addr1.addr().ip().size(), btclite::NetAddr::ip_uint32_size);
    EXPECT_EQ(std::memcmp(addr1.addr().ip().data(), ip_none, btclite::NetAddr::ip_byte_size), 0);
    
    for (uint8_t i = 1; i <= btclite::NetAddr::ip_uint32_size; i++)
        addr1.SetByte(i, i);
    addr1.set_port(1234);
    addr1.set_scope_id(1);
    addr1.set_services(2);
    addr1.set_timestamp(3);
      
    btclite::NetAddr addr2(addr1);
    ASSERT_EQ(addr1, addr2);
    
    btclite::NetAddr addr3(std::move(addr1));
    ASSERT_EQ(addr2, addr3);
    
    struct sockaddr_in sock_addr;
    sock_addr.sin_addr.s_addr = inet_addr("192.168.1.1");
    sock_addr.sin_port = htons(1234);
    btclite::NetAddr addr4(sock_addr);
    ASSERT_TRUE(addr4.IsIpv4());
    EXPECT_EQ(addr4.GetIpv4(), sock_addr.sin_addr.s_addr);
    EXPECT_EQ(addr4.port(), sock_addr.sin_port);
    
    struct sockaddr_in6 sock_addr6;
    uint8_t out[btclite::NetAddr::ip_byte_size];
    for (uint8_t i = 1; i <= btclite::NetAddr::ip_uint32_size; i++)
        sock_addr6.sin6_addr.s6_addr[i] = i;
    sock_addr6.sin6_port = htons(1234);
    sock_addr6.sin6_scope_id = 1;
    btclite::NetAddr addr5(sock_addr6);
    ASSERT_TRUE(addr5.IsIpv6());
    addr5.GetIpv6(out);
    EXPECT_EQ(std::memcmp(sock_addr6.sin6_addr.s6_addr, out, sizeof(out)), 0);
    EXPECT_EQ(addr5.port(), sock_addr6.sin6_port);
    EXPECT_EQ(addr5.scope_id(), sock_addr6.sin6_scope_id);
}

TEST(NetAddrTest, OperatorEqual)
{
    btclite::NetAddr addr1, addr2;
    for (uint8_t i = 1; i <= btclite::NetAddr::ip_uint32_size; i++)
        addr1.SetByte(i, i);
    addr1.set_port(1234);
    addr2 = addr1;
    EXPECT_EQ(addr1, addr2);
    
    btclite::NetAddr addr3;
    addr3 = std::move(addr2);
    EXPECT_EQ(addr1, addr3);
}

TEST(NetAddrTest, MethordIsIpv4)
{
    btclite::NetAddr addr;
    addr.SetByte(10, 0xff);
    addr.SetByte(11, 0xff);

    EXPECT_TRUE(addr.IsIpv4());
    EXPECT_FALSE(addr.IsIpv6());    
}

TEST(NetAddrTest, MethordIsIpv6)
{
    btclite::NetAddr addr;
    
    for (uint8_t i = 1; i <= btclite::NetAddr::ip_uint32_size; i++)
        addr.SetByte(i, i);   
    EXPECT_FALSE(addr.IsIpv4());
    EXPECT_TRUE(addr.IsIpv6());
}

TEST(NetAddrTest, MethordSetByte)
{
    btclite::NetAddr addr;
    
    for (int i = 0; i < btclite::NetAddr::ip_byte_size; i++)
        addr.SetByte(i, i);
    for (int i = 0; i < btclite::NetAddr::ip_byte_size; i++)
        ASSERT_EQ(addr.GetByte(i), i);
}

TEST(NetAddrTest, MethordSetNByte)
{
    btclite::NetAddr addr;
    
    addr.SetNByte(pch_ipv4, sizeof(pch_ipv4));
    EXPECT_TRUE(addr.IsIpv4());
}

TEST(NetAddrTest, MethordSetIpv4)
{
    btclite::NetAddr addr;
    
    addr.SetIpv4(inet_addr("192.168.1.2"));
    ASSERT_TRUE(addr.IsValid());
    ASSERT_TRUE(addr.IsIpv4());
    ASSERT_EQ(addr.GetByte(12), 192);
    ASSERT_EQ(addr.GetByte(13), 168);
    ASSERT_EQ(addr.GetByte(14), 1);
    ASSERT_EQ(addr.GetByte(15), 2);
}

TEST(NetAddrTest, MethordGetIpv4)
{
    btclite::NetAddr addr;
    
    addr.SetIpv4(inet_addr("192.168.1.1"));
    ASSERT_EQ(addr.GetIpv4(), inet_addr("192.168.1.1"));
}

TEST(NetAddrTest, MethordSetIpv6)
{
    btclite::NetAddr addr;
    uint8_t buf[sizeof(struct in6_addr)];
    
    inet_pton(AF_INET6, "0001:0203:0405:0607:0809:0A0B:0C0D:0E0F", buf);
    addr.SetIpv6(buf);
    ASSERT_TRUE(addr.IsValid());
    for (int i = 0; i < btclite::NetAddr::ip_byte_size; i++)
        ASSERT_EQ(addr.GetByte(i), i);
}

TEST(NetAddrTest, MethordGetIpv6)
{
    btclite::NetAddr addr;
    uint8_t out[btclite::NetAddr::ip_byte_size];
    
    addr.SetIpv4(inet_addr("192.168.1.1"));
    ASSERT_EQ(addr.GetIpv6(out), -1);
    
    uint8_t buf[sizeof(struct in6_addr)];
    inet_pton(AF_INET6, "0001:0203:0405:0607:0809:0A0B:0C0D:0E0F", buf);
    addr.SetIpv6(buf);
    addr.GetIpv6(out);
    ASSERT_EQ(std::memcmp(buf, out, btclite::NetAddr::ip_byte_size), 0);
}

TEST(NetAddrTest, Properties)
{
    btclite::NetAddr addr;
    
    EXPECT_FALSE(addr.IsValid());
    
    addr.SetIpv4(inet_addr("10.0.0.1"));
    EXPECT_TRUE(addr.IsRFC1918());
    addr.SetIpv4(inet_addr("192.168.1.1"));
    EXPECT_TRUE(addr.IsRFC1918());
    addr.SetIpv4(inet_addr("172.31.255.255"));
    EXPECT_TRUE(addr.IsRFC1918());
    
    addr.SetIpv4(inet_addr("198.18.0.0"));
    EXPECT_TRUE(addr.IsRFC2544());
    addr.SetIpv4(inet_addr("198.19.255.255"));
    EXPECT_TRUE(addr.IsRFC2544());
    
    addr.SetIpv4(inet_addr("100.64.0.0"));
    EXPECT_TRUE(addr.IsRFC6598());
    addr.SetIpv4(inet_addr("100.127.255.255"));
    EXPECT_TRUE(addr.IsRFC6598());
    
    addr.SetIpv4(inet_addr("192.0.2.0"));
    EXPECT_TRUE(addr.IsRFC5737());
    addr.SetIpv4(inet_addr("198.51.100.0"));
    EXPECT_TRUE(addr.IsRFC5737());
    addr.SetIpv4(inet_addr("203.0.113.0"));
    EXPECT_TRUE(addr.IsRFC5737());
    
    addr.SetIpv4(inet_addr("169.254.1.1"));
    EXPECT_TRUE(addr.IsRFC3927());
    
    uint8_t buf[sizeof(struct in6_addr)];
    inet_pton(AF_INET6, "2001:0DB8::", buf);
    addr.SetIpv6(buf);
    EXPECT_TRUE(addr.IsRFC3849());
    EXPECT_FALSE(addr.IsValid());
    
    inet_pton(AF_INET6, "2002::1", buf);
    addr.SetIpv6(buf);
    EXPECT_TRUE(addr.IsRFC3964());
    
    inet_pton(AF_INET6, "FC00::", buf);
    addr.SetIpv6(buf);
    EXPECT_TRUE(addr.IsRFC4193());
    
    inet_pton(AF_INET6, "2001::2", buf);
    addr.SetIpv6(buf);
    EXPECT_TRUE(addr.IsRFC4380());
    
    inet_pton(AF_INET6, "2001:10::", buf);
    addr.SetIpv6(buf);
    EXPECT_TRUE(addr.IsRFC4843());
    
    inet_pton(AF_INET6, "64:FF9B::", buf);
    addr.SetIpv6(buf);
    EXPECT_TRUE(addr.IsRFC6052());
    
    inet_pton(AF_INET6, "::FFFF:0:0:0", buf);
    addr.SetIpv6(buf);
    EXPECT_TRUE(addr.IsRFC6145());
    
    addr.SetIpv4(inet_addr("127.0.0.1"));
    EXPECT_TRUE(addr.IsLocal());
    inet_pton(AF_INET6, "::1", buf);
    addr.SetIpv6(buf);
    EXPECT_TRUE(addr.IsLocal());
    
    addr.SetIpv4(inet_addr("8.8.8.8"));
    EXPECT_TRUE(addr.IsRoutable());
    inet_pton(AF_INET6, "2001::1", buf);
    addr.SetIpv6(buf);
    EXPECT_TRUE(addr.IsRoutable());
    
    inet_pton(AF_INET6, "FD6B:88C0:8724:edb1:8e4:3588:e546:35ca", buf);
    addr.SetIpv6(buf);
    EXPECT_TRUE(addr.IsInternal());
    EXPECT_FALSE(addr.IsValid());
    
    for (int i = 0; i < 7; i++)
        addr.SetByte(i, 0);
    addr.SetByte(7, 0xff);
    addr.SetByte(8, 0xff);
    EXPECT_FALSE(addr.IsValid());
    addr.SetIpv4(0);
    EXPECT_FALSE(addr.IsValid());
    addr.SetIpv4(INADDR_NONE);
    EXPECT_FALSE(addr.IsValid());
}

TEST(NetAddrTest, MethordToSockAddr)
{
    btclite::NetAddr addr;
    struct sockaddr_storage sock_addr;
    socklen_t len;
    bool ret;
    
    std::memset(&sock_addr, 0, sizeof(sock_addr));
    addr.SetIpv4(inet_addr("192.168.1.1"));
    addr.set_port(1234);
    
    len = 0;
    ret = addr.ToSockAddr(reinterpret_cast<struct sockaddr*>(&sock_addr), &len);
    ASSERT_FALSE(ret);
    len = sizeof(struct sockaddr_in);
    ret = addr.ToSockAddr(reinterpret_cast<struct sockaddr*>(&sock_addr), &len);
    ASSERT_TRUE(ret);
    
    struct sockaddr_in *sock_addr4 = reinterpret_cast<struct sockaddr_in*>(&sock_addr);
    EXPECT_EQ(sock_addr4->sin_family, AF_INET);
    EXPECT_EQ(sock_addr4->sin_addr.s_addr, addr.GetIpv4());
    EXPECT_EQ(sock_addr4->sin_port, htons(addr.port()));
    
    
    uint8_t buf[sizeof(struct in6_addr)];
    inet_pton(AF_INET6, "0001:0203:0405:0607:0809:0A0B:0C0D:0E0F", buf);
    addr.SetIpv6(buf);
    addr.set_scope_id(3);
    std::memset(&sock_addr, 0, sizeof(sock_addr));
    
    ret = addr.ToSockAddr(reinterpret_cast<struct sockaddr*>(&sock_addr), &len);
    ASSERT_FALSE(ret);
    len = sizeof(struct sockaddr_in6);
    ret = addr.ToSockAddr(reinterpret_cast<struct sockaddr*>(&sock_addr), &len);
    ASSERT_TRUE(ret);
    
    uint8_t out[btclite::NetAddr::ip_byte_size];
    addr.GetIpv6(out);
    struct sockaddr_in6 *sock_addr6 = reinterpret_cast<struct sockaddr_in6*>(&sock_addr);
    EXPECT_EQ(sock_addr6->sin6_family, AF_INET6);
    EXPECT_EQ(std::memcmp(sock_addr6->sin6_addr.s6_addr, out, btclite::NetAddr::ip_byte_size), 0);
    EXPECT_EQ(sock_addr6->sin6_port, htons(addr.port()));
    EXPECT_EQ(sock_addr6->sin6_scope_id, addr.scope_id());
}
