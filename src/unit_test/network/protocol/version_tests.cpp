#include <gtest/gtest.h>

#include "protocol/version.h"
#include "network_address.h"


using namespace btclite::network::protocol;

TEST(VersionTest, Serialize)
{
    std::vector<uint8_t> vec;
    ByteSink<std::vector<uint8_t> > byte_sink(vec);
    ByteSource<std::vector<uint8_t> > byte_source(vec);
    btclite::network::NetAddr addr_recv(0x1234, kNodeNetwork, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x1, 0x2, 0x3, 0x4},
    8333);
    btclite::network::NetAddr addr_from(0x5678, kNodeNetwork, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x1, 0x2, 0x3, 0x5},
    8333);
    
    MessageHeader header1, header2;
    header1.set_magic(0x12345678);
    header1.set_command(std::string(kMsgVersion));
    header1.set_payload_length(1000);
    header1.set_checksum(0x12345678);
    
    Version msg_version1, msg_version2;
    msg_version1.set_version(kProtocolVersion);
    msg_version1.set_services(kNodeNetwork);
    msg_version1.set_timestamp(0x1234);
    msg_version1.set_addr_recv(std::move(addr_recv));
    msg_version1.set_addr_from(std::move(addr_from));
    msg_version1.set_nonce(0x5678);
    msg_version1.set_user_agent(std::move(std::string("/btclite:0.1.0/")));
    msg_version1.set_start_height(1000);
    msg_version1.set_relay(true);
    
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

TEST(VersionTest, SerializedSize)
{
    Version msg_version;
    
    msg_version.set_user_agent("/btclite:0.1.0/");
    EXPECT_EQ(msg_version.SerializedSize(), 113);
}
