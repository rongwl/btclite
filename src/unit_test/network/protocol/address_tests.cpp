#include <gtest/gtest.h>

#include "protocol/address.h"

using namespace btclite::network::protocol;


TEST(NetAddrTest, Serialize)
{
    std::vector<uint8_t> vec;
    ByteSink<std::vector<uint8_t> > byte_sink(vec);
    ByteSource<std::vector<uint8_t> > byte_source(vec);
    NetAddr addr1 = { 0x1234, kNodeNetwork, 
                      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x1, 0x2, 0x3, 0x4},
                      8333}, addr2;
    
    addr1.Serialize(byte_sink);
    addr2.Deserialize(byte_source);
    EXPECT_EQ(addr1.timestamp, addr2.timestamp);
    EXPECT_EQ(addr1.services, addr2.services);
    EXPECT_EQ(addr1.ip, addr2.ip);
    EXPECT_EQ(addr1.port, addr2.port);
}
