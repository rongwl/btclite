#include <gtest/gtest.h>

#include "protocol/reject.h"
#include "stream.h"
#include "random.h"


using namespace btclite::network::protocol;

TEST(RejectTest, Serialize)
{
    std::vector<uint8_t> vec;
    ByteSink<std::vector<uint8_t> > byte_sink(vec);
    ByteSource<std::vector<uint8_t> > byte_source(vec);
    Reject msg1(kMsgVersion, kRejectDuplicate, "Duplicate version message", 
                btclite::utility::random::GetUint256()), msg2;
    
    ASSERT_NE(msg1, msg2);
    msg1.Serialize(byte_sink);
    msg2.Deserialize(byte_source);
    EXPECT_EQ(msg1, msg2);
}

TEST(RejectTest, SerializedSize)
{
    Reject msg(kMsgVersion, kRejectDuplicate, "Duplicate version message", 
                btclite::utility::random::GetUint256());
    EXPECT_EQ(msg.SerializedSize(), 67);
}

TEST(RejectTest, ConstructFromRaw)
{
    MemOstream os;
    Reject msg1(kMsgVersion, kRejectDuplicate, "Duplicate version message", 
                btclite::utility::random::GetUint256());
    
    os << msg1;
    Reject msg2(os.vec().data(), msg1.SerializedSize());
    EXPECT_EQ(msg1, msg2);
}
