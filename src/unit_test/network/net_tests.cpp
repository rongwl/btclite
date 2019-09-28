#include <gtest/gtest.h>
#include <cstdint>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "constants.h"
#include "net.h"
#include "network/include/params.h"
#include "message_types/version.h"


TEST(MessageHeaderTest, GetAndSet)
{
    MessageHeader header;
    header.set_magic(kMainMagic);
    header.set_command(kMsgVersion);
    header.set_payload_length(kMaxMessageSize);
    header.set_checksum(0x12345678);
    
    EXPECT_EQ(kMainMagic, header.magic());
    EXPECT_EQ(kMsgVersion, header.command());
    EXPECT_EQ(kMaxMessageSize, header.payload_length());
    EXPECT_EQ(0x12345678, header.checksum());
    
    header.set_command("foo");
    EXPECT_EQ("foo", header.command());
}

TEST(MessageHeaderTest, ValidateHeader)
{
    MessageHeader header;
    std::vector<uint32_t> magic_vec = { kMainMagic, kTestnetMagic, kRegtestMagic };
    std::vector<std::string> command_vec = { kMsgVersion };
    
    for (auto command : command_vec) {
        header.set_magic(kMainMagic);
        header.set_command(command);
        header.set_payload_length(kMaxMessageSize);
        ASSERT_TRUE(header.IsValid());
    }       
    
    header.set_magic(0);    
    ASSERT_FALSE(header.IsValid());
    
    for (int i = 1; i < magic_vec.size(); i++) {
        header.set_magic(magic_vec[i]);
        ASSERT_FALSE(header.IsValid());
    }
    
    header.set_magic(kMainMagic);
    header.set_command("foo");
    EXPECT_FALSE(header.IsValid());
    
    header.set_magic(kMainMagic);
    header.set_command(kMsgVersion);
    header.set_payload_length(kMaxMessageSize+1);
    EXPECT_FALSE(header.IsValid());
}

TEST(MessageHeaderTest, Constructor1)
{
    MessageHeader header;
    
    ASSERT_FALSE(header.IsValid());
}

TEST(MessageHeaderTest, Constructor2)
{
    MessageHeader header(kMainMagic);
    ASSERT_FALSE(header.IsValid());
    EXPECT_EQ(kMainMagic, header.magic());
}

TEST(TestMessageHeader, Constructor3)
{
    MessageHeader header1(kMainMagic, kMsgVersion, kMaxMessageSize, 0x12345678);
    ASSERT_TRUE(header1.IsValid());
    EXPECT_EQ(kMainMagic, header1.magic());
    EXPECT_EQ(kMsgVersion, header1.command());
    EXPECT_EQ(kMaxMessageSize, header1.payload_length());
    EXPECT_EQ(0x12345678, header1.checksum());
    
    MessageHeader header2(0x123, "foo", kMaxMessageSize, 0x12345678);
    ASSERT_FALSE(header2.IsValid());
    EXPECT_EQ("foo", header2.command());
}

TEST(MessageHeaderTest, Constructor4)
{
    const MessageHeader header1(kMainMagic, kMsgVersion, kMaxMessageSize, 0x12345678);
    MessageHeader header2(header1);
    ASSERT_TRUE(header2.IsValid());
    EXPECT_EQ(kMainMagic, header2.magic());
    EXPECT_EQ(kMsgVersion, header2.command());
    EXPECT_EQ(kMaxMessageSize, header2.payload_length());
    EXPECT_EQ(0x12345678, header2.checksum());
    EXPECT_TRUE(header1 == header2);
}

TEST(MessageHeaderTest, Constructor5)
{
    MessageHeader header1(kMainMagic, kMsgVersion, kMaxMessageSize, 0x12345678);
    MessageHeader header2(std::move(header1));
    ASSERT_TRUE(header2.IsValid());
    EXPECT_EQ(kMainMagic, header2.magic());
    EXPECT_EQ(kMsgVersion, header2.command());
    EXPECT_EQ(kMaxMessageSize, header2.payload_length());
    EXPECT_EQ(0x12345678, header2.checksum());
}

TEST(MessageHeaderTest, OperatorEqual)
{
    MessageHeader header1(kMainMagic, kMsgVersion, kMaxMessageSize, 0x12345678);
    MessageHeader header2(header1);
    EXPECT_TRUE(header1 == header2);
    
    header2.set_magic(kTestnetMagic);
    EXPECT_TRUE(header1 != header2);
    
    header2.set_magic(header1.magic());
    header2.set_command("foo");
    EXPECT_TRUE(header1 != header2);
    
    header2.set_command(header1.command());
    header2.set_payload_length(123);
    EXPECT_TRUE(header1 != header2);
    
    header2.set_payload_length(header1.payload_length());
    header2.set_checksum(123);
    EXPECT_TRUE(header1 != header2);
}


TEST(LocalNetConfigTest, Constructor)
{
    LocalNetConfig config;
    EXPECT_EQ(config.local_services(), kNodeNetwork | kNodeNetworkLimited);
}

TEST(LocalNetConfigTest, GetAndSetLocalServiecs)
{
    LocalNetConfig config;
    config.set_local_services(kNodeNetwork);
    EXPECT_EQ(config.local_services(), kNodeNetwork);
}

TEST(LocalNetConfigTest, ValidateLocalAddrs)
{
    LocalNetConfig config;
    btclite::network::NetAddr addr;
    
    ASSERT_TRUE(config.LookupLocalAddrs());
    EXPECT_TRUE(config.IsLocal(config.local_addrs().front()));
    EXPECT_TRUE(config.IsLocal(config.local_addrs().back()));
    
    addr.SetIpv4(inet_addr("1.2.3.4"));
    EXPECT_FALSE(config.IsLocal(addr));  
}
