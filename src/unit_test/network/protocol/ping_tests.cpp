#include <gtest/gtest.h>

#include "protocol/ping.h"


using namespace btclite::network::protocol;

TEST(PingTest, Serialize)
{
    std::vector<uint8_t> vec;
    ByteSink<std::vector<uint8_t> > byte_sink(vec);
    ByteSource<std::vector<uint8_t> > byte_source(vec);
    Ping msg_ping1(0x1122334455667788), msg_ping2;
    
    ASSERT_NE(msg_ping1, msg_ping2);
    msg_ping1.Serialize(byte_sink);
    msg_ping2.Deserialize(byte_source);
    EXPECT_EQ(msg_ping1, msg_ping2);
}

TEST(PingTest, ConstructFromRaw)
{
    std::vector<uint8_t> vec;
    ByteSink<std::vector<uint8_t> > byte_sink(vec);
    ByteSource<std::vector<uint8_t> > byte_source(vec);
    Ping msg_ping1(0x1122334455667788);
    
    msg_ping1.Serialize(byte_sink);
    Ping msg_ping2(vec.data());
    EXPECT_EQ(msg_ping1, msg_ping2);
}
