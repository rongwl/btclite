#include "protocol/pong_tests.h"
#include "stream.h"


TEST_F(PongTest, Constructor)
{
    EXPECT_EQ(pong1_.nonce(), 0);
    EXPECT_EQ(pong2_.nonce(), nonce_);
}

TEST_F(PongTest, OperatorEqual)
{
    EXPECT_NE(pong1_, pong2_);
    pong1_.set_nonce(pong2_.nonce());
    EXPECT_EQ(pong1_, pong2_);
}

TEST_F(PongTest, Serialize)
{
    std::vector<uint8_t> vec;
    ByteSink<std::vector<uint8_t> > byte_sink(vec);
    ByteSource<std::vector<uint8_t> > byte_source(vec);
    
    pong2_.Serialize(byte_sink);
    pong1_.Deserialize(byte_source);
    EXPECT_EQ(pong1_, pong2_);
}

TEST_F(PongTest, SerializedSize)
{
    MemOstream ms;
    
    ms << pong2_;
    EXPECT_EQ(pong2_.SerializedSize(), ms.vec().size());
}
