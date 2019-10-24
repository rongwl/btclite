#include "block_sync_tests.h"


TEST_F(FixtureBlockSyncTest, GetSyncState)
{
    BlockSyncState *state1 = block_sync_.GetSyncState(1);
    BlockSyncState *state2 = block_sync_.GetSyncState(2);
    BlockSyncState *state3 = block_sync_.GetSyncState(3);
    ASSERT_TRUE(state1 && state2 && state3);
    EXPECT_EQ(state1->node_addr(), addr1_);
    EXPECT_EQ(state2->node_addr(), addr2_);
    EXPECT_EQ(state3->node_addr(), addr3_);
}

TEST_F(FixtureBlockSyncTest, EraseSyncState)
{
    block_sync_.EraseSyncState(1);
    ASSERT_TRUE(!block_sync_.GetSyncState(1) &&
                block_sync_.GetSyncState(2) &&
                block_sync_.GetSyncState(3));
    
    block_sync_.EraseSyncState(2);
    ASSERT_TRUE(!block_sync_.GetSyncState(1) &&
                !block_sync_.GetSyncState(2) &&
                block_sync_.GetSyncState(3));
    
    block_sync_.EraseSyncState(3);
    ASSERT_TRUE(!block_sync_.GetSyncState(1) &&
                !block_sync_.GetSyncState(2) &&
                !block_sync_.GetSyncState(3));
}

TEST_F(FixtureBlockSyncTest, ShouldUpdateTime)
{
    EXPECT_FALSE(block_sync_.ShouldUpdateTime(1));
    EXPECT_FALSE(block_sync_.ShouldUpdateTime(2));
    EXPECT_FALSE(block_sync_.ShouldUpdateTime(3));
    
    BlockSyncState *state = block_sync_.GetSyncState(3);
    ASSERT_NE(state, nullptr);
    state->mutable_basic_state()->set_connected(true);
    EXPECT_TRUE(block_sync_.ShouldUpdateTime(3));
}

TEST_F(FixtureBlockSyncTest, Misbehaving)
{
    BlockSyncState *state = block_sync_.GetSyncState(2);
    
    ASSERT_NE(state, nullptr);
    block_sync_.Misbehaving(2, kDefaultBanscoreThreshold - 1);
    ASSERT_FALSE(state->basic_state().should_ban());
    block_sync_.Misbehaving(2, 1);
    ASSERT_TRUE(state->basic_state().should_ban());
}
