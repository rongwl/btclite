#include <gtest/gtest.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "block_sync.h"


class BlockSyncTest : public ::testing::Test {
protected:
    void SetUp() override 
    {
        addr1_.SetIpv4(inet_addr("1.1.1.1"));
        block_sync_.AddSyncState(1, addr1_, "");
        addr2_.SetIpv4(inet_addr("1.1.1.2"));
        block_sync_.AddSyncState(2, addr2_, "");
        addr3_.SetIpv4(inet_addr("1.1.1.3"));
        block_sync_.AddSyncState(3, addr3_, "");
    }
    
    BlockSync block_sync_;
    btclite::network::NetAddr addr1_;
    btclite::network::NetAddr addr2_;
    btclite::network::NetAddr addr3_;
};
