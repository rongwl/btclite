#include <gtest/gtest.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "bandb.h"
#include "p2p.h"
#include "utiltime.h"

TEST(BanDbTest, Constructor)
{
    BanDb ban_db(fs::path("/foo"));
    ASSERT_EQ(ban_db.path_ban_list(), fs::path("/foo") / "banlist.dat");
    EXPECT_FALSE(ban_db.dirty());
}

TEST(BanDbTest, AddBanAddr)
{
    btclite::NetAddr addr;
    addr.SetIpv4(inet_addr("1.2.3.4"));
    BanDb ban_db(fs::path("/tmp"));
    
    ASSERT_TRUE(ban_db.Add(addr, BanDb::NodeMisbehaving));
    auto it = ban_db.ban_map().map().find(SubNet(addr).ToString());
    ASSERT_NE(it, ban_db.ban_map().map().end());
    EXPECT_EQ(it->second.ban_reason(), BanDb::NodeMisbehaving);
    EXPECT_TRUE(ban_db.dirty());
    
    ban_db.set_dirty(false);
    ASSERT_TRUE(ban_db.Erase(addr, false));
    it = ban_db.ban_map().map().find(SubNet(addr).ToString());
    EXPECT_EQ(it, ban_db.ban_map().map().end());
    EXPECT_TRUE(ban_db.dirty());
    EXPECT_FALSE(ban_db.Erase(addr, false));
}

TEST(BanDbTest, ClearBanDb)
{
    BanDb ban_db(fs::path("/tmp"));
    btclite::NetAddr addr;
    
    for (int i = 1; i < 10; i++) {
        std::string ip = "1.2.3." + std::to_string(i);
        addr.SetIpv4(inet_addr(ip.c_str()));
        ASSERT_TRUE(ban_db.Add(addr, BanDb::NodeMisbehaving));
    }
    EXPECT_EQ(ban_db.Size(), 9);
    ban_db.Clear();
    EXPECT_EQ(ban_db.Size(), 0);
}

TEST(BanDbTest, SweepBannedAddrs)
{
    btclite::NetAddr addr;
    addr.SetIpv4(inet_addr("1.2.3.4"));
    SubNet subnet(addr);
    proto_banmap::BanEntry ban_entry;
    ban_entry.set_version(1);
    ban_entry.set_create_time(Time::GetTimeSeconds()-2);
    ban_entry.set_ban_until(Time::GetTimeSeconds()-1);
    ban_entry.set_ban_reason(BanDb::NodeMisbehaving);
    proto_banmap::BanMap ban_map;
    (*ban_map.mutable_map())[subnet.ToString()] = ban_entry;
    
    BanDb ban_db(fs::path("/tmp"), ban_map);
    ASSERT_EQ(ban_db.path_ban_list(), fs::path("/tmp") / "banlist.dat");
    auto it = ban_db.ban_map().map().find(subnet.ToString());
    ASSERT_NE(it, ban_db.ban_map().map().end());
    ASSERT_EQ(it->second.version(), ban_entry.version());
    ASSERT_EQ(it->second.create_time(), ban_entry.create_time());
    ASSERT_EQ(it->second.ban_until(), ban_entry.ban_until());
    ASSERT_EQ(it->second.ban_reason(), ban_entry.ban_reason());
    EXPECT_TRUE(ban_db.dirty());
    ban_db.SweepBanned();
    EXPECT_EQ(ban_db.ban_map().map().find(subnet.ToString()), ban_db.ban_map().map().end());
    
    ASSERT_TRUE(ban_db.Add(subnet, BanDb::NodeMisbehaving));
    ban_db.SweepBanned();
    it = ban_db.ban_map().map().find(subnet.ToString());
    EXPECT_NE(it, ban_db.ban_map().map().end());
    EXPECT_EQ(it->second.ban_reason(), BanDb::NodeMisbehaving);
}

TEST(BanDbTest, DumpAndLoadBanList)
{    
    btclite::NetAddr addr;
    proto_banmap::BanEntry ban_entry;
    proto_banmap::BanMap ban_map;
    char buf[10];
    
    std::memset(buf, 0, sizeof(buf));
    for (int i = 0; i < 10; i++) {        
        std::sprintf(buf, "1.2.3.%d", i);
        addr.SetIpv4(inet_addr(buf));
        SubNet subnet(addr);
        ban_entry.set_version(i);
        ban_entry.set_create_time(Time::GetTimeSeconds());
        ban_entry.set_ban_until(Time::GetTimeSeconds()+kDefaultMisbehavingBantime);
        ban_entry.set_ban_reason(BanDb::ManuallyAdded);
        (*ban_map.mutable_map())[subnet.ToString()] = ban_entry;
    }
    
    BanDb ban_db(fs::path("/tmp"), ban_map);
    ASSERT_EQ(ban_db.Size(), 10);
    ASSERT_TRUE(ban_db.dirty());
    ASSERT_TRUE(ban_db.DumpBanList());
    ASSERT_FALSE(ban_db.LoadBanList());
    ban_db.Clear();
    ASSERT_TRUE(ban_db.LoadBanList());
    ASSERT_EQ(ban_db.Size(), 10);
    ban_map = ban_db.ban_map();
    for (int i = 0; i < 10; i++) {
        auto it = ban_map.map().find("1.2.3." + std::to_string(i) + "/32");
        ASSERT_NE(it, ban_map.map().end());
        EXPECT_EQ(it->second.version(), i);
        EXPECT_EQ(it->second.ban_reason(), BanDb::ManuallyAdded);
    }

    fs::remove(ban_db.path_ban_list());
}

TEST(BanDbTest, AddrIsBanned)
{
    btclite::NetAddr addr;
    proto_banmap::BanEntry ban_entry;
    proto_banmap::BanMap ban_map;
    char buf[10];
    
    std::memset(buf, 0, sizeof(buf));
    for (int i = 0; i < 10; i++) {        
        std::sprintf(buf, "1.2.3.%d", i);
        addr.SetIpv4(inet_addr(buf));
        ban_entry.set_version(i);
        ban_entry.set_create_time(Time::GetTimeSeconds());
        ban_entry.set_ban_until(Time::GetTimeSeconds()+kDefaultMisbehavingBantime);
        ban_entry.set_ban_reason(BanDb::NodeMisbehaving);
        (*ban_map.mutable_map())[SubNet(addr).ToString()] = ban_entry;
    }
        
    BanDb ban_db1(fs::path("/tmp"), ban_map);
    for (int i = 0; i < 10; i++) {
        std::sprintf(buf, "1.2.3.%d", i);
        addr.SetIpv4(inet_addr(buf));
        EXPECT_TRUE(ban_db1.IsBanned(addr));
    }
    
    ban_entry.set_ban_until(ban_entry.ban_until() - kDefaultMisbehavingBantime);
    addr.SetIpv4(inet_addr("1.2.3.4"));
    (*ban_map.mutable_map())[SubNet(addr).ToString()] = ban_entry;
    BanDb ban_db2(fs::path("/tmp"), ban_map);
    EXPECT_FALSE(ban_db2.IsBanned(addr));
    
    addr.SetIpv4(inet_addr("1.2.3.10"));
    EXPECT_FALSE(ban_db2.IsBanned(addr));
}
