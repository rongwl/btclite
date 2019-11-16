#include "protocol/inventory_vector_tests.h"
#include "stream.h"


TEST_F(InvVectTest, Constructor)
{
    EXPECT_EQ(inv_vect1_.type(), 0);
    EXPECT_TRUE(inv_vect1_.hash().IsNull());
    
    EXPECT_EQ(inv_vect2_.type(), type_);
    EXPECT_EQ(inv_vect2_.hash(), hash_);
}

TEST_F(InvVectTest, Serialize)
{
    std::vector<uint8_t> vec;
    ByteSink<std::vector<uint8_t> > byte_sink(vec);
    ByteSource<std::vector<uint8_t> > byte_source(vec);
    
    ASSERT_NE(inv_vect1_, inv_vect2_);
    inv_vect2_.Serialize(byte_sink);
    inv_vect1_.Deserialize(byte_source);
    EXPECT_EQ(inv_vect1_, inv_vect2_);
}

TEST_F(InvVectTest, SerializedSize)
{
    MemOstream ms;
    ms << inv_vect1_;
    EXPECT_EQ(inv_vect1_.SerializedSize(), ms.vec().size());
}

TEST_F(InvVectTest, Clear)
{
    inv_vect2_.Clear();
    EXPECT_EQ(inv_vect1_, inv_vect2_);
}
