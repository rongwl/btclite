#include "version_tests.h"


TEST_F(FixtureVersionTest, Constructor)
{
    EXPECT_EQ(version1_.version(), 0);
    EXPECT_EQ(version1_.services(), 0);
    EXPECT_EQ(version1_.timestamp(), 0);
    EXPECT_EQ(version1_.addr_recv(), btclite::network::NetAddr());
    EXPECT_EQ(version1_.addr_from(), btclite::network::NetAddr());
    EXPECT_EQ(version1_.nonce(), 0);
    EXPECT_EQ(version1_.user_agent(), "");
    EXPECT_EQ(version1_.start_height(), 0);
    EXPECT_EQ(version1_.relay(), 0);
    
    EXPECT_EQ(version2_.version(), version_);
    EXPECT_EQ(version2_.services(), services_);
    EXPECT_EQ(version2_.timestamp(), timestamp_);
    EXPECT_EQ(version2_.addr_recv(), addr_recv_);
    EXPECT_EQ(version2_.addr_from(), addr_from_);
    EXPECT_EQ(version2_.nonce(), nonce_);
    EXPECT_EQ(version2_.user_agent(), user_agent_);
    EXPECT_EQ(version2_.start_height(), start_height_);
    EXPECT_EQ(version2_.relay(), relay_);
    
    EXPECT_EQ(version3_.version(), version_);
    EXPECT_EQ(version3_.services(), services_);
    EXPECT_EQ(version3_.timestamp(), timestamp_);
    EXPECT_EQ(version3_.addr_recv(), addr_recv_);
    EXPECT_EQ(version3_.addr_from(), addr_from_);
    EXPECT_EQ(version3_.nonce(), nonce_);
    EXPECT_EQ(version3_.user_agent(), user_agent_);
    EXPECT_EQ(version3_.start_height(), start_height_);
    EXPECT_EQ(version3_.relay(), relay_);
}

TEST_F(FixtureVersionTest, OperatorEqual)
{
    EXPECT_EQ(version2_, version3_);
    EXPECT_NE(version1_, version2_);
    
    version2_.set_version(kShortIdsBlocksVersion);
    EXPECT_NE(version2_, version3_);
    
    version2_.set_version(version3_.version());
    EXPECT_EQ(version2_, version3_);
    version2_.set_services(kNodeNone);
    EXPECT_NE(version2_, version3_);
    
    version2_.set_services(version3_.services());
    EXPECT_EQ(version2_, version3_);
    version2_.set_timestamp(0x5678);
    EXPECT_NE(version2_, version3_);
    
    version2_.set_timestamp(version3_.timestamp());
    EXPECT_EQ(version2_, version3_);
    version2_.set_addr_recv(std::move(btclite::network::NetAddr()));
    EXPECT_NE(version2_, version3_);
    
    version2_.set_addr_recv(std::move(version3_.addr_recv()));
    EXPECT_EQ(version2_, version3_);
    version2_.set_addr_from(std::move(btclite::network::NetAddr()));
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

TEST_F(FixtureVersionTest, Set)
{
    version1_.set_version(version2_.version());
    version1_.set_services(version2_.services());
    version1_.set_timestamp(version2_.timestamp());
    version1_.set_addr_recv(version2_.addr_recv());
    version1_.set_addr_from(version2_.addr_from());
    version1_.set_nonce(version2_.nonce());
    version1_.set_user_agent(version2_.user_agent());
    version1_.set_start_height(version2_.start_height());
    version1_.set_relay(version2_.relay());
    EXPECT_EQ(version1_, version2_);
    
    version1_.set_addr_recv(btclite::network::NetAddr());
    version1_.set_addr_recv(std::move(btclite::network::NetAddr(version2_.addr_recv())));
    version1_.set_addr_from(btclite::network::NetAddr());
    version1_.set_addr_from(std::move(btclite::network::NetAddr(version2_.addr_from())));
    version1_.set_user_agent("");
    version1_.set_user_agent(std::move(std::string(version2_.user_agent())));
    EXPECT_EQ(version1_, version2_);
}

TEST_F(FixtureVersionTest, Clear)
{
    version2_.Clear();
    EXPECT_EQ(version1_, version2_);
}

TEST_F(FixtureVersionTest, IsValid)
{
    EXPECT_FALSE(version1_.IsValid());
    
    version2_.set_version(kMinPeerProtoVersion);
    EXPECT_TRUE(version2_.IsValid());
    
    version2_.set_version(kAddrTimeVersion);
    EXPECT_FALSE(version2_.IsValid());
    
    version2_.set_version(kMinPeerProtoVersion);
    EXPECT_TRUE(version2_.IsValid());
    version2_.set_addr_recv(std::move(btclite::network::NetAddr()));
    EXPECT_FALSE(version2_.IsValid());
    
    version2_.set_addr_recv(std::move(btclite::network::NetAddr(addr_recv_)));
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

TEST_F(FixtureVersionTest, Serialize)
{
    std::vector<uint8_t> vec;
    ByteSink<std::vector<uint8_t> > byte_sink(vec);
    ByteSource<std::vector<uint8_t> > byte_source(vec);    
    MessageHeader header1(0x12345678, kMsgVersion, 1000, 0x12345678), header2;
    
    header1.Serialize(byte_sink);
    version2_.Serialize(byte_sink);
    header2.Deserialize(byte_source);
    version1_.Deserialize(byte_source);
    EXPECT_EQ(header1, header2);
    EXPECT_EQ(version1_, version2_);
    
    version1_.Clear();
    version2_.set_version(kBip31Version);
    version2_.Serialize(byte_sink);
    version1_.Deserialize(byte_source);
    EXPECT_FALSE(version1_.relay());
}

TEST(VersionTest, SerializedSize)
{
    Version msg_version;
    
    msg_version.set_user_agent("/btclite:0.1.0/");
    EXPECT_EQ(msg_version.SerializedSize(), 113);
}
