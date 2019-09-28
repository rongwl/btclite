#include <gtest/gtest.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "block_sync.h"


TEST(BlockSyncTest, Modify)
{
    BlockSync block_sync;
    btclite::network::NetAddr addr;
    
    addr.SetIpv4(inet_addr("1.1.1.1"));
    block_sync.AddSyncState(1, addr, "");
    ASSERT_NE(block_sync.GetSyncState(1), nullptr);
    block_sync.EraseSyncState(1);
    EXPECT_EQ(block_sync.GetSyncState(1), nullptr);
}
