#include "reject_tests.h"


TEST_F(FixtureRejectTest, Constructor)
{
    EXPECT_EQ(reject1_.message(), "");
    EXPECT_EQ(reject1_.ccode(), 0);
    EXPECT_EQ(reject1_.reason(), "");
    EXPECT_TRUE(reject1_.data().IsNull());
    
    EXPECT_EQ(reject2_.message(), message_);
    EXPECT_EQ(reject2_.ccode(), ccode_);
    EXPECT_EQ(reject2_.reason(), reason_);
    EXPECT_EQ(reject2_.data(), data_);
    
    EXPECT_EQ(reject3_.message(), message_);
    EXPECT_EQ(reject3_.ccode(), ccode_);
    EXPECT_EQ(reject3_.reason(), reason_);
    EXPECT_EQ(reject3_.data(), data_);
}

TEST_F(FixtureRejectTest, OperatorEqual)
{
    EXPECT_EQ(reject2_, reject3_);
    EXPECT_NE(reject1_, reject2_);
    
    reject2_.set_message("foo");
    EXPECT_NE(reject2_, reject3_);
    
    reject2_.set_message(std::move(reject3_.message()));
    EXPECT_EQ(reject2_, reject3_);
    reject2_.set_ccode(kRejectMalformed);
    EXPECT_NE(reject2_, reject3_);
    
    reject2_.set_ccode(reject3_.ccode());
    EXPECT_EQ(reject2_, reject3_);
    reject2_.set_reason("bar");
    EXPECT_NE(reject2_, reject3_);
    
    reject2_.set_reason(std::move(reject3_.reason()));
    EXPECT_EQ(reject2_, reject3_);
    reject2_.set_data(btclite::utility::random::GetUint256());
    EXPECT_NE(reject2_, reject3_);
    
    reject2_.set_message(kMsgVersion);
    reject3_.set_message(kMsgVersion);
    EXPECT_EQ(reject2_, reject3_);
}

TEST_F(FixtureRejectTest, Set)
{
    reject1_.set_message(reject2_.message());
    reject1_.set_ccode(reject2_.ccode());
    reject1_.set_reason(reject2_.reason());
    reject1_.set_data(reject2_.data());
    EXPECT_EQ(reject1_, reject2_);
    
    reject1_.set_message("");
    reject1_.set_message(std::move(reject2_.message()));
    reject1_.set_reason("");
    reject1_.set_reason(std::move(reject2_.reason()));
    EXPECT_EQ(reject1_, reject2_);
}

TEST_F(FixtureRejectTest, Clear)
{
    reject2_.Clear();
    EXPECT_EQ(reject1_, reject2_);
}

TEST_F(FixtureRejectTest, IsValid)
{
    EXPECT_FALSE(reject1_.IsValid());
    reject1_.set_ccode(kRejectInvalid);
    EXPECT_TRUE(reject1_.IsValid());
}

TEST_F(FixtureRejectTest, Serialize)
{
    std::vector<uint8_t> vec;
    ByteSink<std::vector<uint8_t> > byte_sink(vec);
    ByteSource<std::vector<uint8_t> > byte_source(vec);
    
    reject2_.Serialize(byte_sink);
    reject1_.Deserialize(byte_source);
    EXPECT_EQ(reject1_, reject2_);
    
    reject1_.Clear();
    reject2_.set_message(kMsgVersion);
    reject2_.Serialize(byte_sink);
    reject1_.Deserialize(byte_source);
    EXPECT_EQ(reject1_, reject2_);
}

TEST_F(FixtureRejectTest, SerializedSize)
{
    EXPECT_EQ(reject2_.SerializedSize(), 43);
    
    reject2_.set_message(kMsgVersion);
    EXPECT_EQ(reject2_.SerializedSize(), 13);
}
