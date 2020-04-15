#include <gtest/gtest.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "net_base.h"
#include "network/include/params.h"


namespace btclite {
namespace unit_test {

using namespace network;

TEST(NetBaseTest, LookupHost)
{
    NetAddr addr;
    uint8_t buf[sizeof(struct in6_addr)];
    uint8_t out[kIpByteSize];
    
    LookupHost("127.0.0.1", &addr, false, 8333);
    ASSERT_TRUE(addr.IsIpv4());
    EXPECT_EQ(addr.GetIpv4(), inet_addr("127.0.0.1"));
    EXPECT_EQ(addr.port(), 8333);
    
    addr.Clear();
    LookupHost("::FFFF:192.168.1.1", &addr, false, 8333);
    ASSERT_TRUE(addr.IsIpv4());
    EXPECT_EQ(addr.GetIpv4(), inet_addr("192.168.1.1"));
    EXPECT_EQ(addr.port(), 8333);
    
    addr.Clear();
    memset(buf, 0, sizeof(buf));
    memset(out, 0, sizeof(out));
    inet_pton(AF_INET6, "::1", buf);
    LookupHost("::1", &addr, false, 8333);
    ASSERT_TRUE(addr.IsIpv6());
    addr.GetIpv6(out);
    EXPECT_EQ(std::memcmp(buf, out, sizeof(out)), 0);
    EXPECT_EQ(addr.port(), 8333);
    
    addr.Clear();
    LookupHost("bitcoin.org", &addr, true, 8333);
    EXPECT_TRUE(addr.IsValid());
}

TEST(NetBaseTest, LookupSubNet)
{
    SubNet subnet1, subnet2;    
    
    LookupSubNet("1.2.3.4/32", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.3.4/32");
    LookupSubNet("1.2.3.4/31", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.3.4/31");
    LookupSubNet("1.2.3.4/30", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.3.4/30");
    LookupSubNet("1.2.3.4/29", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.3.0/29");
    LookupSubNet("1.2.3.4/28", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.3.0/28");
    LookupSubNet("1.2.3.4/27", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.3.0/27");
    LookupSubNet("1.2.3.4/26", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.3.0/26");
    LookupSubNet("1.2.3.4/25", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.3.0/25");
    LookupSubNet("1.2.3.4/24", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.3.0/24");
    LookupSubNet("1.2.3.4/23", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.2.0/23");
    LookupSubNet("1.2.3.4/22", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.0.0/22");
    LookupSubNet("1.2.3.4/21", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.0.0/21");
    LookupSubNet("1.2.3.4/20", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.0.0/20");
    LookupSubNet("1.2.3.4/19", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.0.0/19");
    LookupSubNet("1.2.3.4/18", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.0.0/18");
    LookupSubNet("1.2.3.4/17", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.0.0/17");
    LookupSubNet("1.2.3.4/16", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.0.0/16");
    LookupSubNet("1.2.3.4/15", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.0.0/15");
    LookupSubNet("1.2.3.4/14", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.0.0.0/14");
    LookupSubNet("1.2.3.4/13", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.0.0.0/13");
    LookupSubNet("1.2.3.4/12", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.0.0.0/12");
    LookupSubNet("1.2.3.4/11", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.0.0.0/11");
    LookupSubNet("1.2.3.4/10", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.0.0.0/10");
    LookupSubNet("1.2.3.4/9", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.0.0.0/9");
    LookupSubNet("1.2.3.4/8", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.0.0.0/8");
    LookupSubNet("1.2.3.4/7", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "0.0.0.0/7");
    LookupSubNet("1.2.3.4/6", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "0.0.0.0/6");
    LookupSubNet("1.2.3.4/5", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "0.0.0.0/5");
    LookupSubNet("1.2.3.4/4", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "0.0.0.0/4");
    LookupSubNet("1.2.3.4/3", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "0.0.0.0/3");
    LookupSubNet("1.2.3.4/2", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "0.0.0.0/2");
    LookupSubNet("1.2.3.4/1", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "0.0.0.0/1");
    LookupSubNet("1.2.3.4/0", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "0.0.0.0/0");
    
    subnet1.Clear();
    LookupSubNet("1.2.3.4/255.255.255.255", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.3.4/32");
    LookupSubNet("1.2.3.4/255.255.255.254", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.3.4/31");
    LookupSubNet("1.2.3.4/255.255.255.252", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.3.4/30");
    LookupSubNet("1.2.3.4/255.255.255.248", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.3.0/29");
    LookupSubNet("1.2.3.4/255.255.255.240", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.3.0/28");
    LookupSubNet("1.2.3.4/255.255.255.224", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.3.0/27");
    LookupSubNet("1.2.3.4/255.255.255.192", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.3.0/26");
    LookupSubNet("1.2.3.4/255.255.255.128", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.3.0/25");
    LookupSubNet("1.2.3.4/255.255.255.0", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.3.0/24");
    LookupSubNet("1.2.3.4/255.255.254.0", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.2.0/23");
    LookupSubNet("1.2.3.4/255.255.252.0", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.0.0/22");
    LookupSubNet("1.2.3.4/255.255.248.0", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.0.0/21");
    LookupSubNet("1.2.3.4/255.255.240.0", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.0.0/20");
    LookupSubNet("1.2.3.4/255.255.224.0", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.0.0/19");
    LookupSubNet("1.2.3.4/255.255.192.0", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.0.0/18");
    LookupSubNet("1.2.3.4/255.255.128.0", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.0.0/17");
    LookupSubNet("1.2.3.4/255.255.0.0", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.0.0/16");
    LookupSubNet("1.2.3.4/255.254.0.0", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.0.0/15");
    LookupSubNet("1.2.3.4/255.252.0.0", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.0.0.0/14");
    LookupSubNet("1.2.3.4/255.248.0.0", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.0.0.0/13");
    LookupSubNet("1.2.3.4/255.240.0.0", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.0.0.0/12");
    LookupSubNet("1.2.3.4/255.224.0.0", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.0.0.0/11");
    LookupSubNet("1.2.3.4/255.192.0.0", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.0.0.0/10");
    LookupSubNet("1.2.3.4/255.128.0.0", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.0.0.0/9");
    LookupSubNet("1.2.3.4/255.0.0.0", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.0.0.0/8");
    LookupSubNet("1.2.3.4/254.0.0.0", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "0.0.0.0/7");
    LookupSubNet("1.2.3.4/252.0.0.0", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "0.0.0.0/6");
    LookupSubNet("1.2.3.4/248.0.0.0", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "0.0.0.0/5");
    LookupSubNet("1.2.3.4/240.0.0.0", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "0.0.0.0/4");
    LookupSubNet("1.2.3.4/224.0.0.0", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "0.0.0.0/3");
    LookupSubNet("1.2.3.4/192.0.0.0", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "0.0.0.0/2");
    LookupSubNet("1.2.3.4/128.0.0.0", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "0.0.0.0/1");
    LookupSubNet("1.2.3.4/0.0.0.0", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "0.0.0.0/0");
    LookupSubNet("1.2.3.4/255.255.232.0", 8333, &subnet1);
    ASSERT_TRUE(subnet1.IsValid());
    EXPECT_EQ(subnet1.ToString(), "1.2.0.0/255.255.232.0");
    
    LookupSubNet("1:2:3:4:5:6:7:8/128", 8333, &subnet2);
    ASSERT_TRUE(subnet2.IsValid());
    EXPECT_EQ(subnet2.ToString(), "1:2:3:4:5:6:7:8/128");
    LookupSubNet("1:2:3:4:5:6:7:8/112", 8333, &subnet2);
    ASSERT_TRUE(subnet2.IsValid());
    EXPECT_EQ(subnet2.ToString(), "1:2:3:4:5:6:7:0/112");
    LookupSubNet("1:2:3:4:5:6:7:8/96", 8333, &subnet2);
    ASSERT_TRUE(subnet2.IsValid());
    EXPECT_EQ(subnet2.ToString(), "1:2:3:4:5:6:0:0/96");
    LookupSubNet("1:2:3:4:5:6:7:8/80", 8333, &subnet2);
    ASSERT_TRUE(subnet2.IsValid());
    EXPECT_EQ(subnet2.ToString(), "1:2:3:4:5:0:0:0/80");
    LookupSubNet("1:2:3:4:5:6:7:8/64", 8333, &subnet2);
    ASSERT_TRUE(subnet2.IsValid());
    EXPECT_EQ(subnet2.ToString(), "1:2:3:4:0:0:0:0/64");
    LookupSubNet("1:2:3:4:5:6:7:8/48", 8333, &subnet2);
    ASSERT_TRUE(subnet2.IsValid());
    EXPECT_EQ(subnet2.ToString(), "1:2:3:0:0:0:0:0/48");
    LookupSubNet("1:2:3:4:5:6:7:8/32", 8333, &subnet2);
    ASSERT_TRUE(subnet2.IsValid());
    EXPECT_EQ(subnet2.ToString(), "1:2:0:0:0:0:0:0/32");
    LookupSubNet("1:2:3:4:5:6:7:8/16", 8333, &subnet2);
    ASSERT_TRUE(subnet2.IsValid());
    EXPECT_EQ(subnet2.ToString(), "1:0:0:0:0:0:0:0/16");
    LookupSubNet("1:2:3:4:5:6:7:8/0", 8333, &subnet2);
    ASSERT_TRUE(subnet2.IsValid());
    EXPECT_EQ(subnet2.ToString(), "0:0:0:0:0:0:0:0/0");
    
    subnet2.Clear();
    LookupSubNet("1:2:3:4:5:6:7:8/ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff", 8333, &subnet2);
    ASSERT_TRUE(subnet2.IsValid());
    EXPECT_EQ(subnet2.ToString(), "1:2:3:4:5:6:7:8/128");
    LookupSubNet("1:2:3:4:5:6:7:8/ffff:ffff:ffff:ffff:ffff:ffff:ffff:0", 8333, &subnet2);
    ASSERT_TRUE(subnet2.IsValid());
    EXPECT_EQ(subnet2.ToString(), "1:2:3:4:5:6:7:0/112");
    LookupSubNet("1:2:3:4:5:6:7:8/ffff:ffff:ffff:ffff:ffff:ffff::", 8333, &subnet2);
    ASSERT_TRUE(subnet2.IsValid());
    EXPECT_EQ(subnet2.ToString(), "1:2:3:4:5:6:0:0/96");
    LookupSubNet("1:2:3:4:5:6:7:8/ffff:ffff:ffff:ffff:ffff::", 8333, &subnet2);
    ASSERT_TRUE(subnet2.IsValid());
    EXPECT_EQ(subnet2.ToString(), "1:2:3:4:5:0:0:0/80");
    LookupSubNet("1:2:3:4:5:6:7:8/ffff:ffff:ffff:ffff::", 8333, &subnet2);
    ASSERT_TRUE(subnet2.IsValid());
    EXPECT_EQ(subnet2.ToString(), "1:2:3:4:0:0:0:0/64");
    LookupSubNet("1:2:3:4:5:6:7:8/ffff:ffff:ffff::", 8333, &subnet2);
    ASSERT_TRUE(subnet2.IsValid());
    EXPECT_EQ(subnet2.ToString(), "1:2:3:0:0:0:0:0/48");
    LookupSubNet("1:2:3:4:5:6:7:8/ffff:ffff::", 8333, &subnet2);
    ASSERT_TRUE(subnet2.IsValid());
    EXPECT_EQ(subnet2.ToString(), "1:2:0:0:0:0:0:0/32");
    LookupSubNet("1:2:3:4:5:6:7:8/ffff::", 8333, &subnet2);
    ASSERT_TRUE(subnet2.IsValid());
    EXPECT_EQ(subnet2.ToString(), "1:0:0:0:0:0:0:0/16");
    LookupSubNet("1:2:3:4:5:6:7:8/::", 8333, &subnet2);
    ASSERT_TRUE(subnet2.IsValid());
    EXPECT_EQ(subnet2.ToString(), "0:0:0:0:0:0:0:0/0");
    LookupSubNet("1:2:3:4:5:6:7:8/ffff:ffff:ffff:fffe:ffff:ffff:ffff:ff0f", 8333, &subnet2);
    ASSERT_TRUE(subnet2.IsValid());
    EXPECT_EQ(subnet2.ToString(), "1:2:3:4:5:6:7:8/ffff:ffff:ffff:fffe:ffff:ffff:ffff:ff0f");
}

} // namespace unit_test
} // namespace btclite
