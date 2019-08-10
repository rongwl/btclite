#include <gtest/gtest.h>
#include <cstdint>
#include <string>
#include <vector>

#include "constants.h"
#include "net.h"
#include "network/include/params.h"
#include "message_types/version.h"


TEST(MessageHeaderTest, MethodGetSet)
{
    MessageHeader header;
    header.set_magic(main_magic);
    header.set_command(btc_message::Version::command);
    header.set_payload_length(max_message_size);
    header.set_checksum(0x12345678);
    
    EXPECT_EQ(main_magic, header.magic());
    EXPECT_EQ(btc_message::Version::command, header.command());
    EXPECT_EQ(max_message_size, header.payload_length());
    EXPECT_EQ(0x12345678, header.checksum());
    
    header.set_command("foo");
    EXPECT_EQ("foo", header.command());
}

TEST(MessageHeaderTest, MethodIsValid)
{
    MessageHeader header;
    std::vector<uint32_t> magic_vec = { main_magic, testnet_magic, regtest_magic };
    std::vector<std::string> command_vec = { btc_message::Version::command };
    
    for (auto command : command_vec) {
        header.set_magic(main_magic);
        header.set_command(command);
        header.set_payload_length(max_message_size);
        ASSERT_TRUE(header.IsValid());
    }       
    
    header.set_magic(0);    
    ASSERT_FALSE(header.IsValid());
    
    for (int i = 1; i < magic_vec.size(); i++) {
        header.set_magic(magic_vec[i]);
        ASSERT_FALSE(header.IsValid());
    }
    
    header.set_magic(main_magic);
    header.set_command("foo");
    EXPECT_FALSE(header.IsValid());
    
    header.set_magic(main_magic);
    header.set_command(btc_message::Version::command);
    header.set_payload_length(max_message_size+1);
    EXPECT_FALSE(header.IsValid());
}

TEST(MessageHeaderTest, Constructor1)
{
    MessageHeader header;
    
    ASSERT_FALSE(header.IsValid());
}

TEST(MessageHeaderTest, Constructor2)
{
    MessageHeader header(main_magic);
    ASSERT_FALSE(header.IsValid());
    EXPECT_EQ(main_magic, header.magic());
}

TEST(TestMessageHeader, Constructor3)
{
    MessageHeader header1(main_magic, btc_message::Version::command, max_message_size, 0x12345678);
    ASSERT_TRUE(header1.IsValid());
    EXPECT_EQ(main_magic, header1.magic());
    EXPECT_EQ(btc_message::Version::command, header1.command());
    EXPECT_EQ(max_message_size, header1.payload_length());
    EXPECT_EQ(0x12345678, header1.checksum());
    
    MessageHeader header2(0x123, "foo", max_message_size, 0x12345678);
    ASSERT_FALSE(header2.IsValid());
    EXPECT_EQ("foo", header2.command());
}

TEST(MessageHeaderTest, Constructor4)
{
    const MessageHeader header1(main_magic, btc_message::Version::command, max_message_size, 0x12345678);
    MessageHeader header2(header1);
    ASSERT_TRUE(header2.IsValid());
    EXPECT_EQ(main_magic, header2.magic());
    EXPECT_EQ(btc_message::Version::command, header2.command());
    EXPECT_EQ(max_message_size, header2.payload_length());
    EXPECT_EQ(0x12345678, header2.checksum());
    EXPECT_TRUE(header1 == header2);
}

TEST(MessageHeaderTest, Constructor5)
{
    MessageHeader header1(main_magic, btc_message::Version::command, max_message_size, 0x12345678);
    MessageHeader header2(std::move(header1));
    ASSERT_TRUE(header2.IsValid());
    EXPECT_EQ(main_magic, header2.magic());
    EXPECT_EQ(btc_message::Version::command, header2.command());
    EXPECT_EQ(max_message_size, header2.payload_length());
    EXPECT_EQ(0x12345678, header2.checksum());
}

TEST(MessageHeaderTest, OperatorEqual)
{
    MessageHeader header1(main_magic, btc_message::Version::command, max_message_size, 0x12345678);
    MessageHeader header2(header1);
    EXPECT_TRUE(header1 == header2);
    
    header2.set_magic(testnet_magic);
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
    EXPECT_EQ(config.local_services(), NODE_NETWORK | NODE_NETWORK_LIMITED);
}

TEST(LocalNetConfigTest, MethodSetLocalServiecs)
{
    LocalNetConfig config;
    config.set_local_services(NODE_NETWORK);
    EXPECT_EQ(config.local_services(), NODE_NETWORK);
}
