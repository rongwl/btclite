#include <gtest/gtest.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "block_sync.h"


namespace btclite {
namespace unit_test {

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
    
    network::BlockSync block_sync_;
    network::NetAddr addr1_;
    network::NetAddr addr2_;
    network::NetAddr addr3_;
};

} // namespace unit_test
} // namespace btclit
