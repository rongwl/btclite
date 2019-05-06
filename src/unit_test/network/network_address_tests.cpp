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
    EXPECT_EQ(addr1.addr().ip().size(), 4);
    
    proto_netaddr::NetAddr proto_addr;
    proto_addr.set_timestamp(1);
    proto_addr.set_services(2);
    for (int i = 1; i <= 4; i++)
        proto_addr.add_ip(i);
    proto_addr.set_port(8388);
    btclite::NetAddr addr2(proto_addr);
    ASSERT_EQ(addr2.addr().timestamp(), proto_addr.timestamp());
    ASSERT_EQ(addr2.addr().services(), proto_addr.services());
    for(int i = 0; i < 4; i++)
        ASSERT_EQ(addr2.addr().ip(i), proto_addr.ip(i));
    ASSERT_EQ(addr2.addr().port(), proto_addr.port());
    
    btclite::NetAddr addr3(std::move(proto_addr));
    ASSERT_EQ(addr3.addr().timestamp(), addr2.addr().timestamp());
    ASSERT_EQ(addr3.addr().services(), addr2.addr().services());
    for(int i = 0; i < 4; i++)
        ASSERT_EQ(addr3.addr().ip(i), addr2.addr().ip(i));
    ASSERT_EQ(addr3.addr().port(), addr2.addr().port());
    
    btclite::NetAddr addr4(addr3);
    ASSERT_EQ(addr3.addr().timestamp(), addr4.addr().timestamp());
    ASSERT_EQ(addr3.addr().services(), addr4.addr().services());
    for(int i = 0; i < 4; i++)
        ASSERT_EQ(addr3.addr().ip(i), addr4.addr().ip(i));
    ASSERT_EQ(addr3.addr().port(), addr4.addr().port());
    
    btclite::NetAddr addr5(std::move(addr4));
    ASSERT_EQ(addr3.addr().timestamp(), addr5.addr().timestamp());
    ASSERT_EQ(addr3.addr().services(), addr5.addr().services());
    for(int i = 0; i < 4; i++)
        ASSERT_EQ(addr3.addr().ip(i), addr5.addr().ip(i));
    ASSERT_EQ(addr3.addr().port(), addr5.addr().port());
}

TEST(NetAddrTest, OperatorEqual)
{
    proto_netaddr::NetAddr proto_addr;
    proto_addr.set_timestamp(1);
    proto_addr.set_services(2);
    for (int i = 1; i <= 4; i++)
        proto_addr.add_ip(i);
    proto_addr.set_port(8388);
    btclite::NetAddr addr1(proto_addr), addr2;
    addr2 = addr1;
    EXPECT_EQ(addr1, addr2);
    
    btclite::NetAddr addr3;
    addr3 = std::move(addr2);
    EXPECT_EQ(addr1, addr3);
}

TEST(NetAddrTest, MethordSetAddr)
{
    proto_netaddr::NetAddr proto_addr;
    proto_addr.set_timestamp(1);
    proto_addr.set_services(2);
    for (int i = 1; i <= 4; i++)
        proto_addr.add_ip(i);
    proto_addr.set_port(8388);
    
    btclite::NetAddr addr1(proto_addr), addr2, addr3;
    addr2.set_addr(proto_addr);
    EXPECT_EQ(addr1, addr2);
    addr3.set_addr(std::move(proto_addr));
    EXPECT_EQ(addr1, addr3);
}

TEST(NetAddrTest, MethordIsIpv4)
{
    proto_netaddr::NetAddr addr;    
    for (int i = 0; i < 4; i++)
        addr.add_ip(0);
    
    addr.set_ip(2, 0xffff0000);
    addr.set_ip(3, inet_addr("127.0.0.1"));
    EXPECT_TRUE(btclite::NetAddr(addr).IsValid());
    EXPECT_TRUE(btclite::NetAddr(addr).IsIpv4());
    EXPECT_FALSE(btclite::NetAddr(addr).IsIpv6());    
}

TEST(NetAddrTest, MethordIsIpv6)
{
    proto_netaddr::NetAddr addr;    
    for (int i = 0; i < 4; i++)
        addr.add_ip(0);
    
    uint8_t buf[sizeof(struct in6_addr)];
    inet_pton(AF_INET6, "ABCD:EF01:2345:6789:ABCD:EF01:2345:6789", buf);
    addr.set_ip(0, *reinterpret_cast<uint32_t*>(buf));
    addr.set_ip(1, *reinterpret_cast<uint32_t*>(buf+4));
    addr.set_ip(2, *reinterpret_cast<uint32_t*>(buf+8));
    addr.set_ip(3, *reinterpret_cast<uint32_t*>(buf+12));
    btclite::NetAddr net_addr(addr);
    
    EXPECT_TRUE(btclite::NetAddr(addr).IsValid());
    EXPECT_FALSE(btclite::NetAddr(addr).IsIpv4());
    EXPECT_TRUE(btclite::NetAddr(addr).IsIpv6());
}

TEST(NetAddrTest, MethordGetByte)
{
    proto_netaddr::NetAddr addr;    
    for (int i = 0; i < 4; i++)
        addr.add_ip(0);
    
    addr.set_ip(2, 0xffff0000);
    addr.set_ip(3, inet_addr("192.168.1.2"));
    btclite::NetAddr net_addr(addr);
    
    ASSERT_EQ(net_addr.GetByte(12), 192);
    ASSERT_EQ(net_addr.GetByte(13), 168);
    ASSERT_EQ(net_addr.GetByte(14), 1);
    ASSERT_EQ(net_addr.GetByte(15), 2);
    
    
    uint8_t buf[sizeof(struct in6_addr)];
    inet_pton(AF_INET6, "0001:0203:0405:0607:0809:0A0B:0C0D:0E0F", buf);
    addr.set_ip(0, *reinterpret_cast<uint32_t*>(buf));
    addr.set_ip(1, *reinterpret_cast<uint32_t*>(buf+4));
    addr.set_ip(2, *reinterpret_cast<uint32_t*>(buf+8));
    addr.set_ip(3, *reinterpret_cast<uint32_t*>(buf+12));
    btclite::NetAddr net_addr2(addr);
    
    for (int i = 0; i < 16; i++)
        ASSERT_EQ(net_addr2.GetByte(i), i);
}

TEST(NetAddrTest, MethordSetByte)
{
    btclite::NetAddr addr;
    
    for (int i = 0; i < 16; i++)
        addr.SetByte(i, i);
    for (int i = 0; i < 16; i++)
        ASSERT_EQ(addr.GetByte(i), i);
}

TEST(NetAddrTest, MethordSetNByte)
{
    btclite::NetAddr addr;
    
    addr.SetNByte(pch_ipv4, sizeof(pch_ipv4));
    EXPECT_TRUE(addr.IsIpv4());
}

TEST(NetAddrTest, MethordGetIpv4)
{
    proto_netaddr::NetAddr addr;
    for (int i = 0; i < 4; i++)
        addr.add_ip(0);
    
    ASSERT_EQ(btclite::NetAddr(addr).GetIpv4(), INADDR_NONE);
    addr.set_ip(2, 0xffff0000);
    addr.set_ip(3, inet_addr("192.168.1.1"));
    ASSERT_EQ(btclite::NetAddr(addr).GetIpv4(), 0x0101A8C0);
}

TEST(NetAddrTest, MethordSetIpv4)
{
    btclite::NetAddr addr;
    
    addr.SetIpv4(inet_addr("192.168.1.2"));
    ASSERT_TRUE(btclite::NetAddr(addr).IsIpv4());
    ASSERT_EQ(addr.GetByte(12), 192);
    ASSERT_EQ(addr.GetByte(13), 168);
    ASSERT_EQ(addr.GetByte(14), 1);
    ASSERT_EQ(addr.GetByte(15), 2);
}

TEST(NetAddrTest, MethordGetIpv6)
{
    btclite::NetAddr addr;
    uint8_t out[16];
    
    addr.SetIpv4(inet_addr("192.168.1.1"));
    ASSERT_EQ(addr.GetIpv6(out), -1);
    
    proto_netaddr::NetAddr proto_addr;
    uint8_t buf[sizeof(struct in6_addr)];
    inet_pton(AF_INET6, "0001:0203:0405:0607:0809:0A0B:0C0D:0E0F", buf);
    proto_addr.add_ip(*reinterpret_cast<uint32_t*>(buf));
    proto_addr.add_ip(*reinterpret_cast<uint32_t*>(buf+4));
    proto_addr.add_ip(*reinterpret_cast<uint32_t*>(buf+8));
    proto_addr.add_ip(*reinterpret_cast<uint32_t*>(buf+12));
    addr.set_addr(std::move(proto_addr));
    addr.GetIpv6(out);
    ASSERT_EQ(std::memcmp(buf, out, 16), 0);
}

TEST(NetAddrTest, MethordSetIpv6)
{
    btclite::NetAddr addr;
    uint8_t buf[sizeof(struct in6_addr)];
    
    inet_pton(AF_INET6, "0001:0203:0405:0607:0809:0A0B:0C0D:0E0F", buf);
    addr.SetIpv6(buf);
    for (int i = 0; i < 16; i++)
        ASSERT_EQ(addr.GetByte(i), i);
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
    
    proto_netaddr::NetAddr proto_addr;
    proto_addr.add_ip(0);
    proto_addr.add_ip(0xFF000000);
    proto_addr.add_ip(0xFF);
    proto_addr.add_ip(inet_addr("192.168.1.1"));
    addr.set_addr(std::move(proto_addr));
    EXPECT_FALSE(addr.IsValid());
    addr.SetIpv4(0);
    EXPECT_FALSE(addr.IsValid());
    addr.SetIpv4(INADDR_NONE);
    EXPECT_FALSE(addr.IsValid());
}
