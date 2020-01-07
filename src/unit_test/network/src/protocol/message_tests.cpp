#include "protocol/message_tests.h"

#include "stream.h"


namespace btclite {
namespace unit_test {

using namespace network::protocol;

TEST_F(MessageHeaderTest, Constructor)
{
    EXPECT_EQ(0, header1_.magic());
    EXPECT_EQ("", header1_.command());
    EXPECT_EQ(0, header1_.payload_length());
    EXPECT_EQ(0, header1_.checksum());
    
    EXPECT_EQ(magic_, header2_.magic());
    EXPECT_EQ(command_, header2_.command());
    EXPECT_EQ(payload_length_, header2_.payload_length());
    EXPECT_EQ(checksum_, header2_.checksum());
    
    EXPECT_EQ(magic_, header3_.magic());
    EXPECT_EQ(command_, header3_.command());
    EXPECT_EQ(payload_length_, header3_.payload_length());
    EXPECT_EQ(checksum_, header3_.checksum());
}

TEST_F(MessageHeaderTest, ConstructFromRaw)
{
    util::MemOstream os;
    os << header2_;
    
    MessageHeader header(os.vec().data());
    EXPECT_EQ(header, header2_);
}

TEST_F(MessageHeaderTest, OperatorEqual)
{
    EXPECT_NE(header1_, header2_);
    EXPECT_EQ(header2_, header3_);
    
    header3_.set_magic(kTestnetMagic);
    EXPECT_NE(header2_, header3_);
    
    header3_.set_magic(header2_.magic());
    header3_.set_command("foo");
    EXPECT_NE(header2_, header3_);
    
    header3_.set_command(header2_.command());
    header3_.set_payload_length(123);
    EXPECT_NE(header2_, header3_);
    
    header3_.set_payload_length(header2_.payload_length());
    header3_.set_checksum(123);
    EXPECT_NE(header2_, header3_);
}

TEST_F(MessageHeaderTest, Set)
{
    header1_.set_magic(header2_.magic());
    header1_.set_command(header2_.command());
    header1_.set_payload_length(header2_.payload_length());
    header1_.set_checksum(header2_.checksum());
    
    EXPECT_EQ(header1_, header2_);
    
    header1_.set_command(std::string("foofoofoofoofoo"));
    EXPECT_EQ("foofoofoofoo", header1_.command());
    
    header1_.set_command(std::move(std::string("barbarbarbar")));    
    EXPECT_EQ("barbarbarbar", header1_.command());
}

TEST_F(MessageHeaderTest, Clear)
{
    header2_.Clear();
    
    EXPECT_EQ(header1_, header2_);
}

TEST_F(MessageHeaderTest, ValidateHeader)
{
    std::vector<uint32_t> magic_vec = { kMainMagic, kTestnetMagic, kRegtestMagic };
    std::vector<std::string> command_vec = { msg_command::kMsgVersion };
    
    for (auto command : command_vec) {
        header1_.set_magic(kMainMagic);
        header1_.set_command(command);
        header1_.set_payload_length(kMaxMessageSize);
        ASSERT_TRUE(header1_.IsValid());
    }       
    
    header1_.set_magic(0);    
    ASSERT_FALSE(header1_.IsValid());
    
    for (int i = 1; i < magic_vec.size(); i++) {
        header1_.set_magic(magic_vec[i]);
        ASSERT_FALSE(header1_.IsValid());
    }
    
    header1_.set_magic(kMainMagic);
    header1_.set_command("foo");
    EXPECT_FALSE(header1_.IsValid());
    
    header1_.set_magic(kMainMagic);
    header1_.set_command(msg_command::kMsgVersion);
    header1_.set_payload_length(kMaxMessageSize+1);
    EXPECT_FALSE(header1_.IsValid());
}

TEST_F(MessageHeaderTest, Serialize)
{
    std::vector<uint8_t> vec;
    util::ByteSink<std::vector<uint8_t> > byte_sink(vec);
    util::ByteSource<std::vector<uint8_t> > byte_source(vec);
    
    header2_.Serialize(byte_sink);
    header1_.Deserialize(byte_source);
    EXPECT_EQ(header1_, header2_);
}

} // namespace unit_test
} // namespace btclite
