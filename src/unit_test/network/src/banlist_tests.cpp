#include <gtest/gtest.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "banlist.h"
#include "p2p.h"
#include "util_time.h"


namespace btclite {
namespace unit_test {

using namespace network;


TEST(BanListTest, AddBanAddr)
{
    NetAddr addr;
    addr.SetIpv4(inet_addr("1.2.3.4"));
    BanList ban_list;
    
    ASSERT_TRUE(ban_list.Add(addr, BanList::BanReason::kNodeMisbehaving));
    auto ban_map = ban_list.ban_map().map();
    auto it = ban_map.find(SubNet(addr).ToString());
    ASSERT_NE(it, ban_list.ban_map().map().end());
    EXPECT_EQ(it->second.ban_reason(), 
              static_cast<std::underlying_type_t<BanList::BanReason> >(
                  BanList::BanReason::kNodeMisbehaving));
    
    ASSERT_TRUE(ban_list.Erase(addr));
    auto ban_map2 = ban_list.ban_map().map();
    auto it2 = ban_map2.find(SubNet(addr).ToString());
    EXPECT_EQ(it2, ban_list.ban_map().map().end());
    EXPECT_FALSE(ban_list.Erase(addr));
}

TEST(BanListTest, ClearBanList)
{
    BanList ban_list;
    NetAddr addr;
    
    for (int i = 1; i < 10; i++) {
        std::string ip = "1.2.3." + std::to_string(i);
        addr.SetIpv4(inet_addr(ip.c_str()));
        ASSERT_TRUE(ban_list.Add(addr, BanList::BanReason::kNodeMisbehaving));
    }
    EXPECT_EQ(ban_list.Size(), 9);
    ban_list.Clear();
    EXPECT_EQ(ban_list.Size(), 0);
}

TEST(BanListTest, SweepBannedAddrs)
{
    NetAddr addr;
    addr.SetIpv4(inet_addr("1.2.3.4"));
    SubNet subnet(addr);
    proto_banmap::BanEntry ban_entry;
    ban_entry.set_version(1);
    ban_entry.set_create_time(util::GetTimeSeconds()-2);
    ban_entry.set_ban_until(util::GetTimeSeconds()-1);
    ban_entry.set_ban_reason(
        static_cast<std::underlying_type_t<BanList::BanReason> >(
            BanList::BanReason::kNodeMisbehaving));
    proto_banmap::BanMap ban_map;
    (*ban_map.mutable_map())[subnet.ToString()] = ban_entry;
    
    BanList ban_list(ban_map);
    auto it = ban_list.ban_map().map().find(subnet.ToString());
    ASSERT_NE(it, ban_list.ban_map().map().end());
    ASSERT_EQ(it->second.version(), ban_entry.version());
    ASSERT_EQ(it->second.create_time(), ban_entry.create_time());
    ASSERT_EQ(it->second.ban_until(), ban_entry.ban_until());
    ASSERT_EQ(it->second.ban_reason(), ban_entry.ban_reason());
    ban_list.SweepBanned();
    EXPECT_EQ(ban_list.ban_map().map().find(subnet.ToString()), ban_list.ban_map().map().end());
    
    ASSERT_TRUE(ban_list.Add(subnet, BanList::BanReason::kNodeMisbehaving));
    ban_list.SweepBanned();
    it = ban_list.ban_map().map().find(subnet.ToString());
    EXPECT_NE(it, ban_list.ban_map().map().end());
    EXPECT_EQ(it->second.ban_reason(), 
              static_cast<std::underlying_type_t<BanList::BanReason> >(
                  BanList::BanReason::kNodeMisbehaving));
}

TEST(BanListTest, AddrIsBanned)
{
    NetAddr addr;
    proto_banmap::BanEntry ban_entry;
    proto_banmap::BanMap ban_map;
    char buf[10];
    
    std::memset(buf, 0, sizeof(buf));
    for (int i = 0; i < 10; i++) {        
        std::sprintf(buf, "1.2.3.%d", i);
        addr.SetIpv4(inet_addr(buf));
        ban_entry.set_version(i);
        ban_entry.set_create_time(util::GetTimeSeconds());
        ban_entry.set_ban_until(util::GetTimeSeconds()+kDefaultMisbehavingBantime);
        ban_entry.set_ban_reason(
            static_cast<std::underlying_type_t<BanList::BanReason> >(
                BanList::BanReason::kNodeMisbehaving));
        (*ban_map.mutable_map())[SubNet(addr).ToString()] = ban_entry;
    }
    
    BanList ban_list1(ban_map);
    for (int i = 0; i < 10; i++) {
        std::sprintf(buf, "1.2.3.%d", i);
        addr.SetIpv4(inet_addr(buf));
        EXPECT_TRUE(ban_list1.IsBanned(addr));
    }
    
    ban_entry.set_ban_until(ban_entry.ban_until() - kDefaultMisbehavingBantime);
    addr.SetIpv4(inet_addr("1.2.3.4"));
    (*ban_map.mutable_map())[SubNet(addr).ToString()] = ban_entry;
    BanList ban_list2(ban_map);
    EXPECT_FALSE(ban_list2.IsBanned(addr));
    
    addr.SetIpv4(inet_addr("1.2.3.10"));
    EXPECT_FALSE(ban_list2.IsBanned(addr));
}

TEST(BanDbTest, Constructor)
{
    BanDb ban_db(fs::path("/foo"));
    ASSERT_EQ(ban_db.path_ban_list(), fs::path("/foo") / "banlist.dat");
}

TEST(BanDbTest, DumpAndLoadBanList)
{    
    NetAddr addr;
    proto_banmap::BanEntry ban_entry;
    proto_banmap::BanMap ban_map;
    char buf[10];
    
    std::memset(buf, 0, sizeof(buf));
    for (int i = 0; i < 10; i++) {        
        std::sprintf(buf, "1.2.3.%d", i);
        addr.SetIpv4(inet_addr(buf));
        SubNet subnet(addr);
        ban_entry.set_version(i);
        ban_entry.set_create_time(util::GetTimeSeconds());
        ban_entry.set_ban_until(util::GetTimeSeconds()+kDefaultMisbehavingBantime);
        ban_entry.set_ban_reason(
            static_cast<std::underlying_type_t<BanList::BanReason> >(
                BanList::BanReason::kManuallyAdded));
        (*ban_map.mutable_map())[subnet.ToString()] = ban_entry;
    }
    
    BanList ban_list(ban_map);
    BanDb ban_db(fs::path("/tmp"));
    ASSERT_EQ(ban_list.Size(), 10);
    ASSERT_TRUE(ban_db.DumpBanList(ban_list));
    ban_list.Clear();
    ASSERT_TRUE(ban_db.LoadBanList(&ban_list));
    ASSERT_EQ(ban_list.Size(), 10);
    ban_map = ban_list.ban_map();
    for (int i = 0; i < 10; i++) {
        auto it = ban_map.map().find("1.2.3." + std::to_string(i) + "/32");
        ASSERT_NE(it, ban_map.map().end());
        EXPECT_EQ(it->second.version(), i);
        EXPECT_EQ(it->second.ban_reason(), 
                  static_cast<std::underlying_type_t<BanList::BanReason> >(
                      BanList::BanReason::kManuallyAdded));
    }

    fs::remove(ban_db.path_ban_list());
}


} // namespace unit_test
} // namespace btclite
