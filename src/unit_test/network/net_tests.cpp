#include <gtest/gtest.h>
#include <cstdint>
#include <string>
#include <vector>

#include "constants.h"
#include "net.h"

TEST(MessageHeaderTest, MethordGetSet)
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

TEST(MessageHeaderTest, MethordIsValid)
{
    MessageHeader header;
    std::vector<BaseEnv> env_vec = { BaseEnv::mainnet, BaseEnv::testnet, BaseEnv::regtest };
    std::vector<uint32_t> magic_vec = { main_magic, testnet_magic, regtest_magic };
    std::vector<std::string> command_vec = { btc_message::Version::command };
    
    for (int i = 0; i < magic_vec.size(); i++) 
        for (auto command : command_vec) {
            header.set_magic(magic_vec[i]);
            header.set_command(command);
            header.set_payload_length(max_message_size);
            ASSERT_TRUE(header.IsValid(env_vec[i]));
        }
    
    for (auto env : env_vec) {
        header.set_magic(0);
        ASSERT_FALSE(header.IsValid(env));
    }
    for (int i = 1; i < magic_vec.size(); i++) {
        header.set_magic(magic_vec[i]);
        ASSERT_FALSE(header.IsValid(BaseEnv::mainnet));
    }
    
    header.set_magic(main_magic);
    header.set_command("foo");
    EXPECT_FALSE(header.IsValid(BaseEnv::mainnet));
    
    header.set_magic(main_magic);
    header.set_command(btc_message::Version::command);
    header.set_payload_length(max_message_size+1);
    EXPECT_FALSE(header.IsValid(BaseEnv::mainnet));
}

TEST(MessageHeaderTest, Constructor1)
{
    MessageHeader header;
    std::vector<BaseEnv> env_vec = { BaseEnv::mainnet, BaseEnv::testnet, BaseEnv::regtest };
    
    for (auto env : env_vec)
        ASSERT_FALSE(header.IsValid(env));
}

TEST(MessageHeaderTest, Constructor2)
{
    MessageHeader header(main_magic);
    ASSERT_FALSE(header.IsValid(BaseEnv::mainnet));
    EXPECT_EQ(main_magic, header.magic());
}

TEST(TestMessageHeader, Constructor3)
{
    MessageHeader header1(main_magic, btc_message::Version::command, max_message_size, 0x12345678);
    ASSERT_TRUE(header1.IsValid(BaseEnv::mainnet));
    EXPECT_EQ(main_magic, header1.magic());
    EXPECT_EQ(btc_message::Version::command, header1.command());
    EXPECT_EQ(max_message_size, header1.payload_length());
    EXPECT_EQ(0x12345678, header1.checksum());
    
    MessageHeader header2(0x123, "foo", max_message_size, 0x12345678);
    ASSERT_FALSE(header2.IsValid(BaseEnv::mainnet));
    EXPECT_EQ("foo", header2.command());
}

TEST(MessageHeaderTest, Constructor4)
{
    const MessageHeader header1(main_magic, btc_message::Version::command, max_message_size, 0x12345678);
    MessageHeader header2(header1);
    ASSERT_TRUE(header2.IsValid(BaseEnv::mainnet));
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
    ASSERT_TRUE(header2.IsValid(BaseEnv::mainnet));
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


TEST(MessageTest, Constructor1)
{}


TEST(LocalNetConfigTest, Constructor)
{
    LocalNetConfig config;
    EXPECT_EQ(config.local_services(), NODE_NETWORK | NODE_NETWORK_LIMITED);
}

TEST(LocalNetConfigTest, MethordSetLocalServiecs)
{
    LocalNetConfig config;
    config.set_local_services(NODE_NETWORK);
    EXPECT_EQ(config.local_services(), NODE_NETWORK);
}
