#include <gtest/gtest.h>

#include "compact.h"


namespace btclite {
namespace unit_test {

using namespace consensus;

TEST(CompactTest, Constructor)
{
    Compact compact1(0);
    EXPECT_EQ(compact1.normal(), 0);
    EXPECT_FALSE(compact1.overflowed());
    
    Compact compact2(0x123456);
    EXPECT_EQ(compact2.normal(), 0);
    EXPECT_FALSE(compact2.overflowed());
    
    Compact compact3(0x01003456);
    EXPECT_EQ(compact3.normal(), 0);
    EXPECT_FALSE(compact3.overflowed());
    
    Compact compact4(0x02000056);
    EXPECT_EQ(compact4.normal(), 0);
    EXPECT_FALSE(compact4.overflowed());
    
    Compact compact5(0x03000000);
    EXPECT_EQ(compact5.normal(), 0);
    EXPECT_FALSE(compact5.overflowed());
    
    Compact compact6(0x04000000);
    EXPECT_EQ(compact6.normal(), 0);
    EXPECT_FALSE(compact6.overflowed());
    
    Compact compact7(0x00923456);
    EXPECT_EQ(compact7.normal(), 0);
    EXPECT_FALSE(compact7.overflowed());
    
    Compact compact8(0x01803456);
    EXPECT_EQ(compact8.normal(), 0);
    EXPECT_FALSE(compact8.overflowed());
    
    Compact compact9(0x02800056);
    EXPECT_EQ(compact9.normal(), 0);
    EXPECT_FALSE(compact9.overflowed());
    
    Compact compact10(0x03800000);
    EXPECT_EQ(compact10.normal(), 0);
    EXPECT_FALSE(compact10.overflowed());
    
    Compact compact11(0x04800000);
    EXPECT_EQ(compact11.normal(), 0);
    EXPECT_FALSE(compact11.overflowed());
    
    Compact compact12(0x01123456);
    EXPECT_EQ(compact12.compact(), 0x01120000);
    EXPECT_FALSE(compact12.overflowed());
    
    Compact compact13(0x02123456);
    EXPECT_FALSE(compact13.overflowed());
    
    Compact compact14(0x03123456);
    EXPECT_EQ(compact14.compact(), 0x03123456);
    EXPECT_FALSE(compact14.overflowed());
    
    Compact compact15(0x04123456);
    EXPECT_EQ(compact15.compact(), 0x04123456);
    EXPECT_FALSE(compact15.overflowed());
    
    Compact compact16(0x05009234);
    EXPECT_EQ(compact16.compact(), 0x05009234);
    EXPECT_FALSE(compact16.overflowed());
    
    Compact compact17(0x20123456);
    EXPECT_EQ(compact17.compact(), 0x20123456);
    EXPECT_FALSE(compact17.overflowed());
    
    Compact compact18(0xff123456);
    EXPECT_TRUE(compact18.overflowed());
}


} // namespace unit_test
} // namespace btclit
