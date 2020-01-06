#include "circular_buffer_tests.h"


namespace btclite {
namespace unit_test {

TEST_F(CircularBufferTest, Constructor)
{
    EXPECT_EQ(buf0_.capacity(), 3);
    EXPECT_EQ(buf0_.empty(), true);
    EXPECT_EQ(buf0_.size(), 0);
    EXPECT_THROW(buf0_[0], std::out_of_range);
}

TEST_F(CircularBufferTest, PushBack)
{
    EXPECT_FALSE(buf1_.empty());
    EXPECT_EQ(buf1_.size(), 1);
    EXPECT_FALSE(buf2_.empty());
    EXPECT_EQ(buf2_.size(), 2);
    EXPECT_FALSE(buf3_.empty());
    EXPECT_EQ(buf3_.size(), 3);
    
    EXPECT_EQ(buf3_[0], 1);
    EXPECT_EQ(buf3_[1], 2);
    EXPECT_EQ(buf3_[2], 3);
    EXPECT_THROW(buf3_[3], std::out_of_range);
    
    buf3_.push_back(4);
    buf3_.push_back(5);
    EXPECT_EQ(buf3_[0], 3);
    EXPECT_EQ(buf3_[1], 4);
    EXPECT_EQ(buf3_[2], 5);
}

TEST_F(CircularBufferTest, PopFront)
{
    EXPECT_EQ(buf3_.pop_front(), 1);
    EXPECT_EQ(buf3_.size(), 2);
    EXPECT_EQ(buf3_.pop_front(), 2);
    EXPECT_EQ(buf3_.size(), 1);
    EXPECT_EQ(buf3_.pop_front(), 3);
    EXPECT_EQ(buf3_.size(), 0);
    EXPECT_TRUE(buf3_.empty());
}

TEST_F(CircularBufferTest, Assign)
{
    EXPECT_THROW(buf0_[0] = 1, std::out_of_range);
    
    buf3_[0] = 6;
    buf3_[1] = 7;
    buf3_[2] = 8;
    EXPECT_EQ(buf3_[0], 6);
    EXPECT_EQ(buf3_[1], 7);
    EXPECT_EQ(buf3_[2], 8);
}

TEST_F(CircularBufferTest, Exist)
{
    EXPECT_TRUE(buf3_.exist(1));
    EXPECT_TRUE(buf3_.exist(2));
    EXPECT_TRUE(buf3_.exist(3));
    EXPECT_FALSE(buf3_.exist(4));
}

} // namespace unit_test
} // namespace btclit
