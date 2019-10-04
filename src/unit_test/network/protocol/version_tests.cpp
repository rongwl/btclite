#include <gtest/gtest.h>

#include "protocol/version.h"


using namespace btclite::network::protocol;

TEST(VersionTest, Serialize)
{
    std::vector<uint8_t> vec;
    ByteSink<std::vector<uint8_t> > byte_sink(vec);
    ByteSource<std::vector<uint8_t> > byte_source(vec);
    NetAddr addr_recv = { 0x1234, kNodeNetwork, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x1, 0x2, 0x3, 0x4},
    8333 };
    NetAddr addr_from = { 0x5678, kNodeNetwork, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x1, 0x2, 0x3, 0x5},
    8333 };
    Version msg_version1(kProtocolVersion, kNodeNetwork, 0x1234, addr_recv,
                         addr_from, 0x5678, "/btclite:0.1.0/", 1000, true);
    Version msg_version2;
    
    ASSERT_NE(msg_version1, msg_version2);
    msg_version1.Serialize(byte_sink);
    msg_version2.Deserialize(byte_source);
    EXPECT_EQ(msg_version1, msg_version2);
}

TEST(VersionTest, ConstructorFromRaw)
{
    std::vector<uint8_t> vec;
    ByteSink<std::vector<uint8_t> > byte_sink(vec);    
    NetAddr addr_recv = { 0x1234, kNodeNetwork, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x1, 0x2, 0x3, 0x4},
    8333 };
    NetAddr addr_from = { 0x5678, kNodeNetwork, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x1, 0x2, 0x3, 0x5},
    8333 };
    Version msg_version1(kProtocolVersion, kNodeNetwork, 0x1234, addr_recv,
                         addr_from, 0x5678, "/btclite:0.1.0/", 1000, true);
    
    msg_version1.Serialize(byte_sink);
    Version msg_version2(vec.data(), msg_version1.SerializedSize());
    EXPECT_EQ(msg_version1, msg_version2);
}
