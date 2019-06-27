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
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = inet_addr("192.168.1.1");
    sock_addr.sin_port = htons(1234);
    btclite::NetAddr addr4(sock_addr);
    ASSERT_TRUE(addr4.IsIpv4());
    EXPECT_EQ(addr4.GetIpv4(), sock_addr.sin_addr.s_addr);
    EXPECT_EQ(addr4.port(), ntohs(sock_addr.sin_port));
    
    struct sockaddr_in6 sock_addr6;
    uint8_t out[btclite::NetAddr::ip_byte_size];
    sock_addr6.sin6_family = AF_INET6;
    for (uint8_t i = 1; i <= btclite::NetAddr::ip_uint32_size; i++)
        sock_addr6.sin6_addr.s6_addr[i] = i;
    sock_addr6.sin6_port = htons(1234);
    sock_addr6.sin6_scope_id = 1;
    btclite::NetAddr addr5(sock_addr6);
    ASSERT_TRUE(addr5.IsIpv6());
    addr5.GetIpv6(out);
    EXPECT_EQ(std::memcmp(sock_addr6.sin6_addr.s6_addr, out, sizeof(out)), 0);
    EXPECT_EQ(addr5.port(), ntohs(sock_addr6.sin6_port));
    EXPECT_EQ(addr5.scope_id(), sock_addr6.sin6_scope_id);
    
    struct sockaddr_storage *storage_addr = reinterpret_cast<struct sockaddr_storage*>(&sock_addr);
    btclite::NetAddr addr6(*storage_addr);
    ASSERT_TRUE(addr6.IsIpv4());
    EXPECT_EQ(addr6.GetIpv4(), sock_addr.sin_addr.s_addr);
    EXPECT_EQ(addr6.port(), ntohs(sock_addr.sin_port));
    
    storage_addr = reinterpret_cast<struct sockaddr_storage*>(&sock_addr6);
    btclite::NetAddr addr7(*storage_addr);
    ASSERT_TRUE(addr7.IsIpv6());
    addr7.GetIpv6(out);
    EXPECT_EQ(std::memcmp(sock_addr6.sin6_addr.s6_addr, out, sizeof(out)), 0);
    EXPECT_EQ(addr7.port(), ntohs(sock_addr6.sin6_port));
    EXPECT_EQ(addr7.scope_id(), sock_addr6.sin6_scope_id);
    
    inet_pton(AF_INET6, "::ffff:192.168.1.1", sock_addr6.sin6_addr.s6_addr);
    btclite::NetAddr addr8(sock_addr6);
    ASSERT_TRUE(addr8.IsIpv4());
    EXPECT_EQ(addr8.GetIpv4(), inet_addr("192.168.1.1"));
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

TEST(NetAddrTest, MethordFromSockAddr)
{
    struct sockaddr_in sock_addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = inet_addr("192.168.1.1");
    sock_addr.sin_port = htons(1234);
    btclite::NetAddr addr;
    addr.FromSockAddr(reinterpret_cast<const struct sockaddr*>(&sock_addr));
    ASSERT_TRUE(addr.IsIpv4());
    EXPECT_EQ(addr.GetIpv4(), sock_addr.sin_addr.s_addr);
    EXPECT_EQ(addr.port(), ntohs(sock_addr.sin_port));
    
    struct sockaddr_in6 sock_addr6;
    uint8_t out[btclite::NetAddr::ip_byte_size];
    sock_addr6.sin6_family = AF_INET6;
    for (uint8_t i = 1; i <= btclite::NetAddr::ip_uint32_size; i++)
        sock_addr6.sin6_addr.s6_addr[i] = i;
    sock_addr6.sin6_port = htons(1234);
    sock_addr6.sin6_scope_id = 1;
    addr.Clear();
    addr.FromSockAddr(reinterpret_cast<const struct sockaddr*>(&sock_addr6));
    ASSERT_TRUE(addr.IsIpv6());
    addr.GetIpv6(out);
    EXPECT_EQ(std::memcmp(sock_addr6.sin6_addr.s6_addr, out, sizeof(out)), 0);
    EXPECT_EQ(addr.port(), ntohs(sock_addr6.sin6_port));
    EXPECT_EQ(addr.scope_id(), sock_addr6.sin6_scope_id);
    
    inet_pton(AF_INET6, "::ffff:192.168.1.1", sock_addr6.sin6_addr.s6_addr);
    addr.Clear();
    addr.FromSockAddr(reinterpret_cast<const struct sockaddr*>(&sock_addr6));
    ASSERT_TRUE(addr.IsIpv4());
    EXPECT_EQ(addr.GetIpv4(), inet_addr("192.168.1.1"));
}

TEST(NetAddrTest, MethordToString)
{
    btclite::NetAddr addr;
    uint8_t buf[sizeof(struct in6_addr)];
    
    addr.SetIpv4(inet_addr("192.168.1.1"));
    EXPECT_EQ(addr.ToString(), "192.168.1.1");
    inet_pton(AF_INET6, "1:2:3:4:5:6:7:8", buf);
    addr.SetIpv6(buf);
    EXPECT_EQ(addr.ToString(), "1:2:3:4:5:6:7:8");
}

TEST(NetAddrTest, MethordClear)
{
    btclite::NetAddr addr;
    
    addr.SetIpv4(inet_addr("192.168.1.1"));
    addr.Clear();
    ASSERT_EQ(addr.addr().ip_size(), btclite::NetAddr::ip_uint32_size);
    for (int i = 0; i < btclite::NetAddr::ip_byte_size; i++)
        ASSERT_EQ(addr.GetByte(i), 0);
}


TEST(SubNetTest, MethordNetmaskBits)
{
    EXPECT_EQ(SubNet::NetmaskBits(0), 0);
    EXPECT_EQ(SubNet::NetmaskBits(0x80), 1);
    EXPECT_EQ(SubNet::NetmaskBits(0xc0), 2);
    EXPECT_EQ(SubNet::NetmaskBits(0xe0), 3);
    EXPECT_EQ(SubNet::NetmaskBits(0xf0), 4);
    EXPECT_EQ(SubNet::NetmaskBits(0xf8), 5);
    EXPECT_EQ(SubNet::NetmaskBits(0xfc), 6);
    EXPECT_EQ(SubNet::NetmaskBits(0xfe), 7);
    EXPECT_EQ(SubNet::NetmaskBits(0xff), 8);
    
    EXPECT_EQ(SubNet::NetmaskBits(0x81), -1);
    EXPECT_EQ(SubNet::NetmaskBits(0xc3), -1);
}

TEST(SubNetTest, Constructor)
{
    btclite::NetAddr addr, netmask;
    uint8_t mask[16];
    
    std::memset(mask, 0xff, 16);
    addr.SetIpv4(inet_addr("192.168.1.1"));
    SubNet subnet1(addr);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.net_addr(), addr);
    EXPECT_EQ(std::memcmp(subnet1.netmask(), mask, 16), 0);
    
    SubNet subnet2(addr, 24);
    addr.SetIpv4(inet_addr("192.168.1.0"));
    mask[15] = 0;
    ASSERT_TRUE(subnet2.IsValid());
    EXPECT_EQ(subnet2.net_addr(), addr);
    EXPECT_EQ(std::memcmp(subnet2.netmask(), mask, 16), 0);
    
    netmask.SetIpv4(inet_addr("255.255.0.0"));
    SubNet subnet3(addr, netmask);
    addr.SetIpv4(inet_addr("192.168.0.0"));
    mask[14] = 0;
    mask[15] = 0;
    ASSERT_TRUE(subnet3.IsValid());
    EXPECT_EQ(subnet3.net_addr(), addr);
    EXPECT_EQ(std::memcmp(subnet3.netmask(), mask, 16), 0);
    
    for (int i = 0; i < 16; i++) {
        addr.SetByte(i, i);
        mask[i] = 0xff;
    }
    SubNet subnet4(addr, 64);    
    for (int i = 8; i < 16; i++) {
        addr.SetByte(i, 0);
        mask[i] = 0;
    }
    ASSERT_TRUE(subnet4.IsValid());
    EXPECT_EQ(subnet4.net_addr(), addr);
    EXPECT_EQ(std::memcmp(subnet4.netmask(), mask, 16), 0);
    
    for (int i = 0; i < 16; i++) {
        addr.SetByte(i, i);
        if (i < 8) {
            netmask.SetByte(i, 0xff);
            mask[i] = 0xff;
        }
        else {
            netmask.SetByte(i, 0);
            mask[i] = 0;
        }
    }
    SubNet subnet5(addr, netmask);    
    for (int i = 8; i < 16; i++)
        addr.SetByte(i, 0);
    ASSERT_TRUE(subnet5.IsValid());
    EXPECT_EQ(subnet5.net_addr(), addr);
    EXPECT_EQ(std::memcmp(subnet5.netmask(), mask, 16), 0);
}

TEST(SubNetTest, MethordMatch)
{
    btclite::NetAddr addr;
    uint8_t buf[sizeof(struct in6_addr)];
    
    addr.SetIpv4(inet_addr("1.2.3.4"));
    SubNet subnet1(addr, 24);
    EXPECT_TRUE(subnet1.Match(addr));    
    addr.SetIpv4(inet_addr("1.2.2.4"));
    EXPECT_FALSE(subnet1.Match(addr));
    
    addr.SetIpv4(inet_addr("1.2.3.4"));
    SubNet subnet2(addr), subnet3(addr, 32);
    EXPECT_TRUE(subnet2.Match(addr));
    EXPECT_TRUE(subnet3.Match(addr));
    addr.SetIpv4(inet_addr("1.2.2.4"));
    EXPECT_FALSE(subnet2.Match(addr));
    EXPECT_FALSE(subnet3.Match(addr));
    
    inet_pton(AF_INET6, "::ffff:127.0.0.1", buf);
    addr.SetIpv6(buf);
    SubNet subnet4(addr);
    EXPECT_TRUE(subnet4.Match(addr));
    
    inet_pton(AF_INET6, "1:2:3:4:5:6:7:8", buf);
    addr.SetIpv6(buf);
    SubNet subnet5(addr);
    EXPECT_TRUE(subnet5.Match(addr));
    addr.SetByte(15, 9);
    EXPECT_FALSE(subnet5.Match(addr));
    
    inet_pton(AF_INET6, "1:2:3:4:5:6:7:0", buf);
    addr.SetIpv6(buf);
    SubNet subnet6(addr, 112);
    addr.SetByte(14, 111);
    addr.SetByte(15, 222);
    EXPECT_TRUE(subnet6.Match(addr));
    
    addr.SetIpv4(inet_addr("192.168.0.1"));
    SubNet subnet7(addr, 24);
    addr.SetByte(15, 2);
    EXPECT_TRUE(subnet7.Match(addr));
    
    addr.SetIpv4(inet_addr("192.168.0.20"));
    SubNet subnet8(addr, 29);
    addr.SetByte(15, 18);
    EXPECT_TRUE(subnet8.Match(addr));
    
    addr.SetIpv4(inet_addr("1.2.2.110"));
    SubNet subnet9(addr, 31);
    addr.SetByte(15, 111);
    EXPECT_TRUE(subnet9.Match(addr));
    
    addr.SetIpv4(inet_addr("1.2.2.20"));
    SubNet subnet10(addr, 26);
    addr.SetByte(15, 63);
    EXPECT_TRUE(subnet10.Match(addr));
    
    // All-Matching IPv6 Matches arbitrary IPv4 and IPv6
    inet_pton(AF_INET6, "::", buf);
    addr.SetIpv6(buf);
    SubNet subnet11(addr, 0);
    inet_pton(AF_INET6, "1:2:3:4:5:6:7:1234", buf);
    addr.SetIpv6(buf);
    EXPECT_TRUE(subnet11.Match(addr));
    addr.SetIpv4(inet_addr("1.2.3.4"));
    EXPECT_TRUE(subnet11.Match(addr));
    
    // All-Matching IPv4 does not Match IPv6
    addr.SetIpv4(inet_addr("0.0.0.0"));
    SubNet subnet12(addr, 0);
    inet_pton(AF_INET6, "1:2:3:4:5:6:7:1234", buf);
    addr.SetIpv6(buf);
    EXPECT_FALSE(subnet12.Match(addr));
    
    // Invalid subnets Match nothing (not even invalid addresses)
    addr.SetIpv4(inet_addr("1.2.3.4"));
    SubNet subnet13;
    EXPECT_FALSE(subnet13.Match(addr));    
}

TEST(SubNetTest, MethordIsValid)
{
    btclite::NetAddr addr;
    uint8_t buf[sizeof(struct in6_addr)];
    
    addr.SetIpv4(inet_addr("1.2.3.0"));
    SubNet subnet1(addr, 0);
    EXPECT_TRUE(subnet1.IsValid());
    SubNet subnet2(addr, 32);
    EXPECT_TRUE(subnet2.IsValid());
    SubNet subnet3(addr, -1);
    EXPECT_FALSE(subnet3.IsValid());
    SubNet subnet4(addr, 33);
    EXPECT_FALSE(subnet4.IsValid());
    
    inet_pton(AF_INET6, "1:2:3:4:5:6:7:8", buf);
    addr.SetIpv6(buf);
    SubNet subnet5(addr, 0);
    EXPECT_TRUE(subnet5.IsValid());
    SubNet subnet6(addr, 33);
    EXPECT_TRUE(subnet6.IsValid());
    SubNet subnet7(addr, -1);
    EXPECT_FALSE(subnet7.IsValid());
    SubNet subnet8(addr, 128);
    EXPECT_TRUE(subnet8.IsValid());
    SubNet subnet9(addr, 129);
    EXPECT_FALSE(subnet9.IsValid());
}

TEST(SubNetTest, OperatorEqual)
{
    btclite::NetAddr addr, netmask;
    uint8_t buf[sizeof(struct in6_addr)];
    
    addr.SetIpv4(inet_addr("1.2.3.0"));
    SubNet subnet1(addr, 24);
    netmask.SetIpv4(inet_addr("255.255.255.0"));
    SubNet subnet2(addr, netmask);
    EXPECT_EQ(subnet1, subnet2);
    addr.SetIpv4(inet_addr("1.2.4.0"));
    SubNet subnet3(addr, netmask);
    EXPECT_NE(subnet1, subnet3);
    
    inet_pton(AF_INET6, "1:2:3:4:5:6:7:8", buf);
    addr.SetIpv6(buf);
    SubNet subnet4(addr, 64);
    inet_pton(AF_INET6, "ffff:ffff:ffff:ffff::", buf);
    netmask.SetIpv6(buf);
    SubNet subnet5(addr, netmask);
    EXPECT_EQ(subnet4, subnet5);
    inet_pton(AF_INET6, "1:2:3:5:5:6:7:8", buf);
    addr.SetIpv6(buf);
    SubNet subnet6(addr, netmask);
    EXPECT_NE(subnet4, subnet6);
}

TEST(SubNetTest, MethordToString)
{
    btclite::NetAddr addr, netmask;
    uint8_t buf[sizeof(struct in6_addr)];
    
    addr.SetIpv4(inet_addr("1.2.3.4"));
    SubNet subnet1(addr, 32);
    EXPECT_EQ(subnet1.ToString(), "1.2.3.4/32");
    SubNet subnet2(addr, 24);
    EXPECT_EQ(subnet2.ToString(), "1.2.3.0/24");
    netmask.SetIpv4(inet_addr("255.255.232.0"));
    SubNet subnet3(addr, netmask);
    EXPECT_EQ(subnet3.ToString(), "1.2.0.0/255.255.232.0");
    SubNet subnet4(addr, 0);
    EXPECT_EQ(subnet4.ToString(), "0.0.0.0/0");
    
    inet_pton(AF_INET6, "1:2:3:4:5:6:7:8", buf);
    addr.SetIpv6(buf);
    SubNet subnet5(addr, 128);
    EXPECT_EQ(subnet5.ToString(), "1:2:3:4:5:6:7:8/128");
    SubNet subnet6(addr, 64);
    EXPECT_EQ(subnet6.ToString(), "1:2:3:4:0:0:0:0/64");
    inet_pton(AF_INET6, "ffff:ffff:ffff:fffe:ffff:ffff:ffff:ff0f", buf);
    netmask.SetIpv6(buf);
    SubNet subnet7(addr, netmask);
    EXPECT_EQ(subnet7.ToString(), "1:2:3:4:5:6:7:8/ffff:ffff:ffff:fffe:ffff:ffff:ffff:ff0f");
    SubNet subnet8(addr, 0);
    EXPECT_EQ(subnet8.ToString(), "0:0:0:0:0:0:0:0/0");
}
