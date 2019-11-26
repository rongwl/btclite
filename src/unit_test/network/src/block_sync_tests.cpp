#include "block_sync_tests.h"


using namespace btclite::network;

TEST_F(BlockSyncTest, GetSyncState)
{
    btclite::network::NetAddr addr;
    
    ASSERT_TRUE(block_sync_.IsExist(1));
    ASSERT_TRUE(block_sync_.IsExist(2));
    ASSERT_TRUE(block_sync_.IsExist(3));
    
    block_sync_.GetNodeAddr(1, &addr);
    EXPECT_EQ(addr, addr1_);
    block_sync_.GetNodeAddr(2, &addr);
    EXPECT_EQ(addr, addr2_);
    block_sync_.GetNodeAddr(3, &addr);
    EXPECT_EQ(addr, addr3_);
}

TEST_F(BlockSyncTest, EraseSyncState)
{
    block_sync_.EraseSyncState(1);
    ASSERT_TRUE(!block_sync_.IsExist(1) &&
                block_sync_.IsExist(2) &&
                block_sync_.IsExist(3));
    
    block_sync_.EraseSyncState(2);
    ASSERT_TRUE(!block_sync_.IsExist(1) &&
                !block_sync_.IsExist(2) &&
                block_sync_.IsExist(3));
    
    block_sync_.EraseSyncState(3);
    ASSERT_TRUE(!block_sync_.IsExist(1) &&
                !block_sync_.IsExist(2) &&
                !block_sync_.IsExist(3));
}

TEST_F(BlockSyncTest, ShouldUpdateTime)
{
    EXPECT_FALSE(block_sync_.ShouldUpdateTime(1));
    EXPECT_FALSE(block_sync_.ShouldUpdateTime(2));
    EXPECT_FALSE(block_sync_.ShouldUpdateTime(3));
    
    block_sync_.SetConnected(3, true);
    EXPECT_TRUE(block_sync_.ShouldUpdateTime(3));
}

TEST_F(BlockSyncTest, Misbehaving)
{
    bool should_ban = false;
    
    block_sync_.Misbehaving(2, kDefaultBanscoreThreshold - 1);
    ASSERT_TRUE(block_sync_.GetShouldBan(2, &should_ban));
    ASSERT_FALSE(should_ban);
    
    block_sync_.Misbehaving(2, 1);
    ASSERT_TRUE(block_sync_.GetShouldBan(2, &should_ban));
    ASSERT_TRUE(should_ban);
}
