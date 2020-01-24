#include "protocol/ping_tests.h"
#include "stream.h"


namespace btclite {
namespace unit_test {

using namespace network::protocol;

TEST_F(PingTest, Constructor)
{
    EXPECT_EQ(ping1_.nonce(), 0);
    EXPECT_EQ(ping1_.protocol_version(), kProtocolVersion);
    EXPECT_EQ(ping2_.nonce(), nonce_);
    EXPECT_EQ(ping2_.protocol_version(), kProtocolVersion);
    EXPECT_EQ(ping3_.protocol_version(), 0);
}

TEST_F(PingTest, OperatorEqual)
{
    EXPECT_NE(ping1_, ping2_);
    ping1_.set_nonce(ping2_.nonce());
    EXPECT_EQ(ping1_, ping2_);
}

TEST_F(PingTest, Serialize)
{
    std::vector<uint8_t> vec;
    util::ByteSink<std::vector<uint8_t> > byte_sink(vec);
    util::ByteSource<std::vector<uint8_t> > byte_source(vec);
    
    ping2_.Serialize(byte_sink);
    ping1_.Deserialize(byte_source);
    EXPECT_EQ(ping1_, ping2_);
}

TEST_F(PingTest, SerializedSize)
{
    util::MemoryStream ms;
    
    ms << ping2_;
    EXPECT_EQ(ping2_.SerializedSize(), ms.Size());
    EXPECT_EQ(ping3_.SerializedSize(), 0);
}

} // namespace unit_test
} // namespace btclit
