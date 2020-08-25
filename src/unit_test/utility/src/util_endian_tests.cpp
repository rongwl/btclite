#include <util_endian.h>

#include <gtest/gtest.h>


namespace btclite {
namespace unit_test {

using namespace util;

TEST(EndianTest, BigEndian)
{
    uint32_t i = 0x11223344;
    uint8_t a[4];
    
    ToBigEndian(i, a);
    EXPECT_EQ(a[0], 0x11);
    EXPECT_EQ(a[1], 0x22);
    EXPECT_EQ(a[2], 0x33);
    EXPECT_EQ(a[3], 0x44);
    
    i = FromBigEndian<uint32_t>(a);
    EXPECT_EQ(i, 0x11223344);
}

TEST(EndianTest, LittleEndian)
{
    uint32_t i = 0x11223344;
    uint8_t a[4];
    
    ToLittleEndian(i, a);
    EXPECT_EQ(a[0], 0x44);
    EXPECT_EQ(a[1], 0x33);
    EXPECT_EQ(a[2], 0x22);
    EXPECT_EQ(a[3], 0x11);
    
    i = FromLittleEndian<uint32_t>(a);
    EXPECT_EQ(i, 0x11223344);
}


} // namespace unit_test
} // namespace btclit
