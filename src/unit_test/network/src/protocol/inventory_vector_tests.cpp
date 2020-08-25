#include "protocol/inventory_vector_tests.h"
#include "stream.h"


namespace btclite {
namespace unit_test {

using namespace network::protocol;

TEST_F(InvVectTest, Constructor)
{
    EXPECT_EQ(inv_vect1_.type(), 0);
    EXPECT_EQ(inv_vect1_.hash(), crypto::null_hash);
    
    EXPECT_EQ(inv_vect2_.type(), type_);
    EXPECT_EQ(inv_vect2_.hash(), hash_);
}

TEST_F(InvVectTest, Serialize)
{
    std::vector<uint8_t> vec;
    util::ByteSink<std::vector<uint8_t> > byte_sink(vec);
    util::ByteSource<std::vector<uint8_t> > byte_source(vec);
    
    ASSERT_NE(inv_vect1_, inv_vect2_);
    inv_vect2_.Serialize(byte_sink);
    inv_vect1_.Deserialize(byte_source);
    EXPECT_EQ(inv_vect1_, inv_vect2_);
}

TEST_F(InvVectTest, SerializedSize)
{
    util::MemoryStream ms;
    ms << inv_vect1_;
    EXPECT_EQ(inv_vect1_.SerializedSize(), ms.Size());
}

TEST_F(InvVectTest, Clear)
{
    inv_vect2_.Clear();
    EXPECT_EQ(inv_vect1_, inv_vect2_);
}

} // namespace unit_test
} // namespace btclit
