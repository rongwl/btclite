#include "protocol/send_compact_tests.h"
#include "stream.h"


TEST_F(SendCmpctTest, Constructor)
{
    EXPECT_FALSE(send_compact1_.high_bandwidth_mode());
    EXPECT_EQ(send_compact1_.version(), 0);
    
    EXPECT_TRUE(send_compact2_.high_bandwidth_mode());
    EXPECT_EQ(send_compact2_.version(), 1);
}

TEST_F(SendCmpctTest, OperatorEqual)
{
    EXPECT_NE(send_compact1_, send_compact2_);
    
    send_compact1_.set_high_bandwidth_mode(send_compact2_.high_bandwidth_mode());
    send_compact1_.set_version(send_compact2_.version());
    EXPECT_EQ(send_compact1_, send_compact2_);
}

TEST_F(SendCmpctTest, Clear)
{
    send_compact2_.Clear();
    EXPECT_EQ(send_compact1_, send_compact2_);
}

TEST_F(SendCmpctTest, IsValid)
{
    EXPECT_FALSE(send_compact1_.IsValid());
    EXPECT_TRUE(send_compact2_.IsValid());
}

TEST_F(SendCmpctTest, Serialize)
{
    std::vector<uint8_t> vec;
    ByteSink<std::vector<uint8_t> > byte_sink(vec);
    ByteSource<std::vector<uint8_t> > byte_source(vec);
    
    send_compact2_.Serialize(byte_sink);
    send_compact1_.Deserialize(byte_source);
    EXPECT_EQ(send_compact1_, send_compact2_);
}

TEST_F(SendCmpctTest, SerializedSize)
{
    MemOstream ms;
    
    ms << send_compact2_;
    EXPECT_EQ(send_compact2_.SerializedSize(), ms.vec().size());
}
