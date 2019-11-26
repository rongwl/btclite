#include <gtest/gtest.h>

#include "compact.h"


using namespace btclite::chain;

TEST(CompactTest, Constructor)
{
    uint32_t num = 0;
    Compact compact1(num);
    EXPECT_EQ(compact1.normal(), 0);
    EXPECT_FALSE(compact1.overflowed());
    
    num = 0x123456;
    Compact compact2(num);
    EXPECT_EQ(compact2.normal(), 0);
    EXPECT_FALSE(compact2.overflowed());
    
    num = 0x01003456;
    Compact compact3(num);
    EXPECT_EQ(compact3.normal(), 0);
    EXPECT_FALSE(compact3.overflowed());
    
    num = 0x02000056;
    Compact compact4(num);
    EXPECT_EQ(compact4.normal(), 0);
    EXPECT_FALSE(compact4.overflowed());
    
    num = 0x03000000;
    Compact compact5(num);
    EXPECT_EQ(compact5.normal(), 0);
    EXPECT_FALSE(compact5.overflowed());
    
    num = 0x04000000;
    Compact compact6(num);
    EXPECT_EQ(compact6.normal(), 0);
    EXPECT_FALSE(compact6.overflowed());
    
    num = 0x00923456;
    Compact compact7(num);
    EXPECT_EQ(compact7.normal(), 0);
    EXPECT_FALSE(compact7.overflowed());
    
    num = 0x01803456;
    Compact compact8(num);
    EXPECT_EQ(compact8.normal(), 0);
    EXPECT_FALSE(compact8.overflowed());
    
    num = 0x02800056;
    Compact compact9(num);
    EXPECT_EQ(compact9.normal(), 0);
    EXPECT_FALSE(compact9.overflowed());
    
    num = 0x03800000;
    Compact compact10(num);
    EXPECT_EQ(compact10.normal(), 0);
    EXPECT_FALSE(compact10.overflowed());
    
    num = 0x04800000;
    Compact compact11(num);
    EXPECT_EQ(compact11.normal(), 0);
    EXPECT_FALSE(compact11.overflowed());
    
    num = 0x01123456;
    Compact compact12(num);
    EXPECT_EQ(compact12.compact(), 0x01120000);
    EXPECT_FALSE(compact12.overflowed());
    
    num = 0x02123456;
    Compact compact13(num);
    EXPECT_FALSE(compact13.overflowed());
    
    num = 0x03123456;
    Compact compact14(num);
    EXPECT_EQ(compact14.compact(), 0x03123456);
    EXPECT_FALSE(compact14.overflowed());
    
    num = 0x04123456;
    Compact compact15(num);
    EXPECT_EQ(compact15.compact(), 0x04123456);
    EXPECT_FALSE(compact15.overflowed());
    
    num = 0x05009234;
    Compact compact16(num);
    EXPECT_EQ(compact16.compact(), 0x05009234);
    EXPECT_FALSE(compact16.overflowed());
    
    num = 0x20123456;
    Compact compact17(num);
    EXPECT_EQ(compact17.compact(), 0x20123456);
    EXPECT_FALSE(compact17.overflowed());
    
    num = 0xff123456;
    Compact compact18(num);
    EXPECT_TRUE(compact18.overflowed());
}

TEST(CompactTest, Negative)
{
    EXPECT_TRUE(Compact::Negative(0x00800001));
    EXPECT_FALSE(Compact::Negative(0x00800000));
}
