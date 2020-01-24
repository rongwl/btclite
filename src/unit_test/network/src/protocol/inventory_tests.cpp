#include "protocol/inventory_tests.h"
#include "stream.h"


namespace btclite {
namespace unit_test {

using namespace network::protocol;

TEST_F(InvTest, Validate)
{
    EXPECT_FALSE(inv1_.IsValid());
    EXPECT_TRUE(inv2_.IsValid());
}

TEST_F(InvTest, Clear)
{
    inv2_.Clear();
    EXPECT_EQ(inv1_, inv2_);
}

TEST_F(InvTest, Serialize)
{
    std::vector<uint8_t> vec;
    util::ByteSink<std::vector<uint8_t> > byte_sink(vec);
    util::ByteSource<std::vector<uint8_t> > byte_source(vec);
    ASSERT_NE(inv1_, inv2_);
    inv1_.Serialize(byte_sink);
    inv2_.Deserialize(byte_source);
    EXPECT_EQ(inv1_, inv2_);
}

TEST_F(InvTest, SerializedSize)
{
    util::MemoryStream ms;

    ms << inv2_;
    EXPECT_EQ(inv2_.SerializedSize(), ms.Size());
}

} // namespace unit_test
} // namespace btclit
