#include <gtest/gtest.h>

#include "protocol/reject.h"
#include "random.h"


using namespace btclite::network::protocol;

TEST(RejectTest, Serialize)
{
    std::vector<uint8_t> vec;
    ByteSink<std::vector<uint8_t> > byte_sink(vec);
    ByteSource<std::vector<uint8_t> > byte_source(vec);
    Reject msg1, msg2;
    
    msg1.set_message(kMsgVersion);
    msg1.set_ccode(kRejectDuplicate);
    msg1.set_reason(std::move(std::string("Duplicate version message")));
    msg1.set_data(std::move(btclite::utility::random::GetUint256()));
    ASSERT_NE(msg1, msg2);
    msg1.Serialize(byte_sink);
    msg2.Deserialize(byte_source);
    EXPECT_EQ(msg1, msg2);
}

TEST(RejectTest, SerializedSize)
{
    Reject msg;
    
    msg.set_message(kMsgVersion);
    msg.set_ccode(kRejectDuplicate);
    msg.set_reason(std::move(std::string("Duplicate version message")));
    msg.set_data(std::move(btclite::utility::random::GetUint256()));
    EXPECT_EQ(msg.SerializedSize(), 67);
}
