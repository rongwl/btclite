#include "protocol/version_tests.h"

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>


namespace btclite {
namespace unit_test {

using namespace network;
using namespace network::protocol;

TEST_F(VersionTest, Constructor)
{
    EXPECT_EQ(version1_.protocol_version(), 0);
    EXPECT_EQ(version1_.services(), 0);
    EXPECT_EQ(version1_.timestamp(), 0);
    EXPECT_EQ(version1_.addr_recv(), NetAddr());
    EXPECT_EQ(version1_.addr_from(), NetAddr());
    EXPECT_EQ(version1_.nonce(), 0);
    EXPECT_EQ(version1_.user_agent(), "");
    EXPECT_EQ(version1_.start_height(), 0);
    EXPECT_EQ(version1_.relay(), 0);
    
    EXPECT_EQ(version2_.protocol_version(), version_);
    EXPECT_EQ(version2_.services(), services_);
    EXPECT_EQ(version2_.timestamp(), timestamp_);
    EXPECT_EQ(version2_.addr_recv(), addr_recv_);
    EXPECT_EQ(version2_.addr_from(), addr_from_);
    EXPECT_EQ(version2_.nonce(), nonce_);
    EXPECT_EQ(version2_.user_agent(), user_agent_);
    EXPECT_EQ(version2_.start_height(), start_height_);
    EXPECT_EQ(version2_.relay(), relay_);
    
    EXPECT_EQ(version3_.protocol_version(), version_);
    EXPECT_EQ(version3_.services(), services_);
    EXPECT_EQ(version3_.timestamp(), timestamp_);
    EXPECT_EQ(version3_.addr_recv(), addr_recv_);
    EXPECT_EQ(version3_.addr_from(), addr_from_);
    EXPECT_EQ(version3_.nonce(), nonce_);
    EXPECT_EQ(version3_.user_agent(), user_agent_);
    EXPECT_EQ(version3_.start_height(), start_height_);
    EXPECT_EQ(version3_.relay(), relay_);
}

TEST_F(VersionTest, OperatorEqual)
{
    EXPECT_EQ(version2_, version3_);
    EXPECT_NE(version1_, version2_);
    
    version2_.set_protocol_version(kShortIdsBlocksVersion);
    EXPECT_NE(version2_, version3_);
    
    version2_.set_protocol_version(version3_.protocol_version());
    EXPECT_EQ(version2_, version3_);
    version2_.set_services(kNodeNone);
    EXPECT_NE(version2_, version3_);
    
    version2_.set_services(version3_.services());
    EXPECT_EQ(version2_, version3_);
    version2_.set_timestamp(0x5678);
    EXPECT_NE(version2_, version3_);
    
    version2_.set_timestamp(version3_.timestamp());
    EXPECT_EQ(version2_, version3_);
    version2_.set_addr_recv(std::move(NetAddr()));
    EXPECT_NE(version2_, version3_);
    
    version2_.set_addr_recv(std::move(version3_.addr_recv()));
    EXPECT_EQ(version2_, version3_);
    version2_.set_addr_from(std::move(NetAddr()));
    EXPECT_NE(version2_, version3_);
    
    version2_.set_addr_from(version3_.addr_from());
    EXPECT_EQ(version2_, version3_);
    version2_.set_nonce(0x1234);
    EXPECT_NE(version2_, version3_);
    
    version2_.set_nonce(version3_.nonce());
    EXPECT_EQ(version2_, version3_);
    version2_.set_user_agent(std::move(std::string("foo")));
    EXPECT_NE(version2_, version3_);
    
    version2_.set_user_agent(std::move(version3_.user_agent()));
    EXPECT_EQ(version2_, version3_);
    version2_.set_start_height(2000);
    EXPECT_NE(version2_, version3_);
    
    version2_.set_start_height(version3_.start_height());
    EXPECT_EQ(version2_, version3_);
    version3_.set_relay(false);
    EXPECT_NE(version2_, version3_);
}

TEST_F(VersionTest, Set)
{
    version1_.set_protocol_version(version2_.protocol_version());
    version1_.set_services(version2_.services());
    version1_.set_timestamp(version2_.timestamp());
    version1_.set_addr_recv(version2_.addr_recv());
    version1_.set_addr_from(version2_.addr_from());
    version1_.set_nonce(version2_.nonce());
    version1_.set_user_agent(version2_.user_agent());
    version1_.set_start_height(version2_.start_height());
    version1_.set_relay(version2_.relay());
    EXPECT_EQ(version1_, version2_);
    
    version1_.set_addr_recv(NetAddr());
    version1_.set_addr_recv(std::move(NetAddr(version2_.addr_recv())));
    version1_.set_addr_from(NetAddr());
    version1_.set_addr_from(std::move(NetAddr(version2_.addr_from())));
    version1_.set_user_agent("");
    version1_.set_user_agent(std::move(std::string(version2_.user_agent())));
    EXPECT_EQ(version1_, version2_);
}

TEST_F(VersionTest, Clear)
{
    version2_.Clear();
    EXPECT_EQ(version1_, version2_);
}

TEST_F(VersionTest, IsValid)
{
    EXPECT_FALSE(version1_.IsValid());
    
    version2_.set_protocol_version(kMinPeerProtoVersion);
    EXPECT_TRUE(version2_.IsValid());
    
    version2_.set_protocol_version(kUnknownProtoVersion);
    EXPECT_FALSE(version2_.IsValid());
    
    version2_.set_protocol_version(kMinPeerProtoVersion);
    EXPECT_TRUE(version2_.IsValid());
    version2_.set_addr_recv(std::move(NetAddr()));
    EXPECT_FALSE(version2_.IsValid());
    
    version2_.set_addr_recv(std::move(NetAddr(addr_recv_)));
    EXPECT_TRUE(version2_.IsValid());
    version2_.set_services(kNodeNone);
    EXPECT_FALSE(version2_.IsValid());
    
    version2_.set_services(kNodeNetwork);
    EXPECT_TRUE(version2_.IsValid());
    version2_.set_timestamp(0);
    EXPECT_FALSE(version2_.IsValid());
    
    version2_.set_timestamp(0x1234);
    EXPECT_TRUE(version2_.IsValid());
    version2_.set_nonce(0);
    EXPECT_FALSE(version2_.IsValid());
}

TEST_F(VersionTest, Serialize)
{
    std::vector<uint8_t> vec;
    util::ByteSink<std::vector<uint8_t> > byte_sink(vec);
    util::ByteSource<std::vector<uint8_t> > byte_source(vec);    
    MessageHeader header1(0x12345678, msg_command::kMsgVersion, 1000, 0x12345678), header2;
    
    header1.Serialize(byte_sink);
    version2_.Serialize(byte_sink);
    header2.Deserialize(byte_source);
    version1_.Deserialize(byte_source);
    EXPECT_EQ(header1, header2);
    EXPECT_EQ(version1_, version2_);
    
    version1_.Clear();
    version2_.set_protocol_version(kBip31Version);
    version2_.Serialize(byte_sink);
    version1_.Deserialize(byte_source);
    EXPECT_FALSE(version1_.relay());
}

TEST_F(VersionTest, SerializedSize)
{
    util::MemOstream ms;
    
    ms << version2_;
    EXPECT_EQ(version2_.SerializedSize(), ms.vec().size());
}

TEST_F(VersionTest, Received)
{
    //struct event_base *base = nullptr;
    //struct bufferevent *pair[2] = {};
    
    //base = event_base_new();
    //ASSERT_NE(base, nullptr);
    //ASSERT_NE(bufferevent_pair_new(base, BEV_OPT_CLOSE_ON_FREE, pair), 0);
    //bufferevent_setcb(pair_[1], SendCmpctReadCb, NULL, NULL, const_cast<char*>(msg_command::kMsgSendCmpct));
    //bufferevent_enable(pair_[1], EV_READ);
    //auto node = std::make_shared<Node>(pair[0], addr_from_, false);
}

} // namespace unit_test
} // namespace btclit
