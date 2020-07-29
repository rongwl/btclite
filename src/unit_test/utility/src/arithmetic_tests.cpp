#include <gtest/gtest.h>

#include "arithmetic.h"


namespace btclite {
namespace unit_test {

using namespace util;

TEST(Uint256Test, Constructor)
{
    Uint256 hex({ 0x9c, 0x52, 0x4a, 0xdb, 0xcf, 0x56, 0x11, 0x12, 
                  0x2b, 0x29, 0x12, 0x5e, 0x5d, 0x35, 0xd2, 0xd2,
                  0x22, 0x81, 0xaa, 0xb5, 0x33, 0xf0, 0x08, 0x32,
                  0xd5, 0x56, 0xb1, 0xf9, 0xea, 0xe5, 0x1d, 0x7d });
    std::string hex_str = "7D1DE5EAF9B156D53208F033B5AA8122D2d2355d5e12292b121156cfdb4a529c";
    std::string hex_zero = "0x0000000000000000000000000000000000000000000000000000000000000000";
    
    EXPECT_EQ(Uint256(hex_str), hex);
    EXPECT_EQ(Uint256("0x"+hex_str), hex);
    EXPECT_EQ(Uint256("     0x"+hex_str+"     "), hex);
    EXPECT_EQ(Uint256(hex_zero), Uint256());
    EXPECT_EQ(Uint256("0x123"), Uint256({0x23, 0x01}));
    
    // overflow
    EXPECT_EQ(Uint256("0x123"+hex_str), hex);
    
    // invalid input
    EXPECT_DEATH(Uint256("0x123 456"), "");
    EXPECT_DEATH(Uint256("0x123x456"), "");
}

} // namespace unit_test
} // namespace btclite
