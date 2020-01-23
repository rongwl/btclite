#include "protocol/getblocks_tests.h"
#include "stream.h"


namespace btclite {
namespace unit_test {

using namespace network::protocol;

TEST_F(GetBlocksTest, Constructor)
{
    EXPECT_EQ(getblocks1_.version(), network::kUnknownProtoVersion);
    EXPECT_EQ(getblocks1_.hashes(), chain::BlockLocator());
    EXPECT_EQ(getblocks1_.hash_stop(), crypto::Hash256());

    EXPECT_EQ(getblocks2_.version(), network::kInvalidCbNoBanVersion);
    EXPECT_EQ(getblocks2_.hashes(), hashes_);
    EXPECT_EQ(getblocks2_.hash_stop(), hash_stop_);
}

TEST_F(GetBlocksTest, Command)
{
    EXPECT_EQ(getblocks1_.Command(), msg_command::kMsgGetBlocks);
}

TEST_F(GetBlocksTest, Set)
{
    EXPECT_NE(getblocks1_, getblocks2_);
    getblocks1_.set_version(getblocks2_.version());
    getblocks1_.set_hashes(getblocks2_.hashes());
    getblocks1_.set_hash_stop(getblocks2_.hash_stop());
    EXPECT_EQ(getblocks1_, getblocks2_);
}

TEST_F(GetBlocksTest, Clear)
{
    getblocks2_.Clear();
    EXPECT_EQ(getblocks1_, getblocks2_);
}

TEST_F(GetBlocksTest, IsValid)
{
    EXPECT_FALSE(getblocks1_.IsValid());
    EXPECT_TRUE(getblocks2_.IsValid());
}

TEST_F(GetBlocksTest, Serialize)
{
    std::vector<uint8_t> vec;
    util::ByteSink<std::vector<uint8_t> > byte_sink(vec);
    util::ByteSource<std::vector<uint8_t> > byte_source(vec);    
    MessageHeader header1(0x12345678, msg_command::kMsgGetBlocks, 1000, 0x12345678), header2;
    
    header1.Serialize(byte_sink);
    getblocks2_.Serialize(byte_sink);
    header2.Deserialize(byte_source);
    getblocks1_.Deserialize(byte_source);
    EXPECT_EQ(header1, header2);
    EXPECT_EQ(getblocks1_, getblocks2_);
}

TEST_F(GetBlocksTest, SerializedSize)
{
    util::MemOstream ms;
    ms << getblocks2_;
    EXPECT_EQ(getblocks2_.SerializedSize(), ms.vec().size());
}

} // namespace unit_test
} // namespace btclit
