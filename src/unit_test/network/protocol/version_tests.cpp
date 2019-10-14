#include <gtest/gtest.h>

#include "protocol/version.h"
#include "network_address.h"
#include "stream.h"


using namespace btclite::network::protocol;

TEST(VersionTest, Serialize)
{
    std::vector<uint8_t> vec;
    ByteSink<std::vector<uint8_t> > byte_sink(vec);
    ByteSource<std::vector<uint8_t> > byte_source(vec);
    MessageHeader header1(0x12345678, std::string(kMsgVersion), 1000, 0x12345678), header2;
    btclite::network::NetAddr addr_recv(0x1234, kNodeNetwork, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x1, 0x2, 0x3, 0x4},
    8333);
    btclite::network::NetAddr addr_from(0x5678, kNodeNetwork, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x1, 0x2, 0x3, 0x5},
    8333);
    Version msg_version1(kProtocolVersion, kNodeNetwork, 0x1234, addr_recv,
                         addr_from, 0x5678, "/btclite:0.1.0/", 1000, true);
    Version msg_version2;
    
    ASSERT_NE(header1, header2);
    ASSERT_NE(msg_version1, msg_version2);
    header1.Serialize(byte_sink);
    msg_version1.Serialize(byte_sink);
    header2.Deserialize(byte_source);
    msg_version2.Deserialize(byte_source);
    EXPECT_EQ(header1, header2);
    EXPECT_EQ(msg_version1, msg_version2);
    
    msg_version1.set_version(kBip31Version);
    msg_version2.Clear();
    msg_version1.Serialize(byte_sink);
    msg_version2.Deserialize(byte_source);
    EXPECT_FALSE(msg_version2.relay());
}

TEST(VersionTest, ConstructFromRaw)
{
    std::vector<uint8_t> vec;
    ByteSink<std::vector<uint8_t> > byte_sink(vec);    
    btclite::network::NetAddr addr_recv(0x1234, kNodeNetwork, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x1, 0x2, 0x3, 0x4},
    8333);
    btclite::network::NetAddr addr_from(0x5678, kNodeNetwork, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x1, 0x2, 0x3, 0x5},
    8333);
    Version msg_version1(kProtocolVersion, kNodeNetwork, 0x1234, addr_recv,
                         addr_from, 0x5678, "/btclite:0.1.0/", 1000, true);
    
    msg_version1.Serialize(byte_sink);
    Version msg_version2(vec.data(), msg_version1.SerializedSize());
    EXPECT_EQ(msg_version1, msg_version2);    
}
