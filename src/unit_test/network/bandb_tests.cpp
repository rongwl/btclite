#include <gtest/gtest.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "bandb.h"
#include "p2p.h"
#include "util_tests.h"
#include "utiltime.h"

TEST(BanDbTest, Constructor)
{
    BanDb ban_db1;
    ASSERT_EQ(ban_db1.path_ban_list(), DataFiles::PathHome() / "banlist.dat");
    EXPECT_FALSE(ban_db1.dirty());
    
    TestDataFiles data_files(fs::path("/foo"));
    BanDb ban_db2(data_files);
    ASSERT_EQ(ban_db2.path_ban_list(), fs::path("/foo") / "banlist.dat");
    EXPECT_FALSE(ban_db2.dirty());
}

TEST(BanDbTest, MethordAdd)
{
    btclite::NetAddr addr;
    addr.SetIpv4(inet_addr("1.2.3.4"));
    TestDataFiles data_files(fs::path("/tmp"));
    BanDb ban_db1(data_files);
    
    ban_db1.Add(addr, BanDb::NodeMisbehaving);
    auto it = ban_db1.ban_map().map().find(SubNet(addr).ToString());
    ASSERT_NE(it, ban_db1.ban_map().map().end());
    EXPECT_EQ(it->second.ban_reason(), BanDb::NodeMisbehaving);
    EXPECT_TRUE(ban_db1.dirty());
}

TEST(BanDbTest, MethordSweepBanned)
{
    btclite::NetAddr addr;
    addr.SetIpv4(inet_addr("1.2.3.4"));
    TestDataFiles data_files(fs::path("/tmp"));
    SubNet subnet(addr);
    proto_banmap::BanEntry ban_entry;
    ban_entry.set_version(1);
    ban_entry.set_create_time(GetTimeSeconds()-2);
    ban_entry.set_ban_until(GetTimeSeconds()-1);
    ban_entry.set_ban_reason(BanDb::NodeMisbehaving);
    proto_banmap::BanMap ban_map;
    (*ban_map.mutable_map())[subnet.ToString()] = ban_entry;
    
    BanDb ban_db(data_files, ban_map);
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
    
    ban_db.Add(subnet, BanDb::NodeMisbehaving);
    ban_db.SweepBanned();
    it = ban_db.ban_map().map().find(subnet.ToString());
    EXPECT_NE(it, ban_db.ban_map().map().end());
    EXPECT_EQ(it->second.ban_reason(), BanDb::NodeMisbehaving);
}

TEST(BanDbTest, MethordDumpBanList)
{    
    btclite::NetAddr addr;
    TestDataFiles data_files(fs::path("/tmp"));
    proto_banmap::BanEntry ban_entry;
    proto_banmap::BanMap ban_map;
    char buf[10];
    
    std::memset(buf, 0, sizeof(buf));
    for (int i = 0; i < 10; i++) {        
        std::sprintf(buf, "1.2.3.%d", i);
        addr.SetIpv4(inet_addr(buf));
        SubNet subnet(addr);
        ban_entry.set_version(i);
        ban_entry.set_create_time(GetTimeSeconds());
        ban_entry.set_ban_until(GetTimeSeconds()+default_misbehaving_bantime);
        ban_entry.set_ban_reason(BanDb::ManuallyAdded);
        (*ban_map.mutable_map())[subnet.ToString()] = ban_entry;
    }
    
    BanDb ban_db(data_files, ban_map);
    ASSERT_TRUE(ban_db.dirty());
    ban_db.DumpBanList();
    std::fstream fs(ban_db.path_ban_list(), std::ios::in | std::ios::binary);
    ASSERT_TRUE(fs.good());
    ASSERT_FALSE(ban_db.dirty());
    ban_map.Clear();
    ASSERT_TRUE(ban_map.ParseFromIstream(&fs));
    for (int i = 0; i < 10; i++) {
        auto it = ban_map.map().find("1.2.3." + std::to_string(i) + "/32");
        ASSERT_NE(it, ban_map.map().end());
        ASSERT_EQ(it->second.version(), i);
        ASSERT_EQ(it->second.ban_reason(), BanDb::ManuallyAdded);
    }
    fs::remove(ban_db.path_ban_list());    
}
