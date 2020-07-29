#include <gtest/gtest.h>

#include "string_encoding.h"


namespace btclite {
namespace unit_test {

using namespace util;

static const unsigned char hex_expected[65] = {
    0x04, 0x67, 0x8a, 0xfd, 0xb0, 0xfe, 0x55, 0x48, 0x27, 0x19, 0x67, 0xf1, 0xa6, 0x71, 0x30, 0xb7,
    0x10, 0x5c, 0xd6, 0xa8, 0x28, 0xe0, 0x39, 0x09, 0xa6, 0x79, 0x62, 0xe0, 0xea, 0x1f, 0x61, 0xde,
    0xb6, 0x49, 0xf6, 0xbc, 0x3f, 0x4c, 0xef, 0x38, 0xc4, 0xf3, 0x55, 0x04, 0xe5, 0x1e, 0xc1, 0x12,
    0xde, 0x5c, 0x38, 0x4d, 0xf7, 0xba, 0x0b, 0x8d, 0x57, 0x8a, 0x4c, 0x70, 0x2b, 0x6b, 0xf1, 0x1d,
    0x5f
};

TEST(StringEncodingTest, EncodeHex)
{
    EXPECT_EQ(EncodeHex(hex_expected, hex_expected + sizeof(hex_expected)),
              "04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f");                 
    
    EXPECT_EQ(EncodeHex(hex_expected, hex_expected + 5, true),
        "04 67 8a fd b0");

    EXPECT_EQ(EncodeHex(hex_expected, hex_expected, true), "");
    
    std::array<uint8_t, 10> zero_arr = {};
    EXPECT_EQ(EncodeHex(zero_arr.begin(), zero_arr.end()), "00000000000000000000");
}

TEST(StringEncodingTest, DecodeHex)
{
    std::vector<uint8_t> expected(hex_expected, hex_expected + sizeof(hex_expected));
    
    // Basic test vector
    EXPECT_EQ(expected, DecodeHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f"));
    
    // Spaces between bytes must be supported
    std::vector<uint8_t> result = DecodeHex("12 34 56 78");
    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], 0x12);
    EXPECT_EQ(result[1], 0x34);
    EXPECT_EQ(result[2], 0x56);
    EXPECT_EQ(result[3], 0x78);
    
    // Leading space must be supported (used in CDBEnv::Salvage)
    result = DecodeHex(" 89 34 56 78");
    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], 0x89);
    EXPECT_EQ(result[1], 0x34);
    EXPECT_EQ(result[2], 0x56);
    EXPECT_EQ(result[3], 0x78);
    
    // skip ending space
    result = DecodeHex("89 34 56 7 8   ");
    ASSERT_EQ(result.size(), 5);
    EXPECT_EQ(result[0], 0x89);
    EXPECT_EQ(result[1], 0x34);
    EXPECT_EQ(result[2], 0x56);
    EXPECT_EQ(result[3], 0x07);
    EXPECT_EQ(result[4], 0x08);
    

    // Stop parsing at invalid value
    result = DecodeHex("1234 invalid 1234");
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x12);
    EXPECT_EQ(result[1], 0x34);
}

TEST(StringEncodingTest, DecodeInt32)
{
    int32_t n;
    
    // Valid values
    EXPECT_TRUE(DecodeInt32("1234", nullptr));
    EXPECT_TRUE(DecodeInt32("0", &n) && n == 0);
    EXPECT_TRUE(DecodeInt32("1234", &n) && n == 1234);
    EXPECT_TRUE(DecodeInt32("01234", &n) && n == 1234); // no octal
    EXPECT_TRUE(DecodeInt32("2147483647", &n) && n == 2147483647);
    EXPECT_TRUE(DecodeInt32("-2147483648", &n) && n == (-2147483647 - 1)); // (-2147483647 - 1) equals INT_MIN
    EXPECT_TRUE(DecodeInt32("-1234", &n) && n == -1234);
    
    // Invalid values
    EXPECT_FALSE(DecodeInt32("", &n));
    EXPECT_FALSE(DecodeInt32(" 1", &n)); // no padding inside
    EXPECT_FALSE(DecodeInt32("1 ", &n));
    EXPECT_FALSE(DecodeInt32("1a", &n));
    EXPECT_FALSE(DecodeInt32("aap", &n));
    EXPECT_FALSE(DecodeInt32("0x1", &n)); // no hex
    EXPECT_FALSE(DecodeInt32("0x1", &n)); // no hex
    const char test_bytes[] = {'1', 0, '1'};
    std::string teststr(test_bytes, sizeof(test_bytes));
    EXPECT_FALSE(DecodeInt32(teststr, &n)); // no embedded NULs
    
    // Overflow and underflow
    EXPECT_FALSE(DecodeInt32("-2147483649", nullptr));
    EXPECT_FALSE(DecodeInt32("2147483648", nullptr));
    EXPECT_FALSE(DecodeInt32("-32482348723847471234", nullptr));
    EXPECT_FALSE(DecodeInt32("32482348723847471234", nullptr));
}

TEST(StringEncodingTest, DecodeInt64)
{
    int64_t n;
    
    // Valid values
    EXPECT_TRUE(DecodeInt64("1234", nullptr));
    EXPECT_TRUE(DecodeInt64("0", &n) && n == 0LL);
    EXPECT_TRUE(DecodeInt64("1234", &n) && n == 1234LL);
    EXPECT_TRUE(DecodeInt64("01234", &n) && n == 1234LL); // no octal
    EXPECT_TRUE(DecodeInt64("2147483647", &n) && n == 2147483647LL);
    EXPECT_TRUE(DecodeInt64("-2147483648", &n) && n == -2147483648LL);
    EXPECT_TRUE(DecodeInt64("9223372036854775807", &n) && n == (int64_t)9223372036854775807);
    EXPECT_TRUE(DecodeInt64("-9223372036854775808", &n) && n == (int64_t)-9223372036854775807-1);
    EXPECT_TRUE(DecodeInt64("-1234", &n) && n == -1234LL);
    
    // Invalid values
    EXPECT_FALSE(DecodeInt64("", &n));
    EXPECT_FALSE(DecodeInt64(" 1", &n)); // no padding inside
    EXPECT_FALSE(DecodeInt64("1 ", &n));
    EXPECT_FALSE(DecodeInt64("1a", &n));
    EXPECT_FALSE(DecodeInt64("aap", &n));
    EXPECT_FALSE(DecodeInt64("0x1", &n)); // no hex
    const char test_bytes[] = {'1', 0, '1'};
    std::string teststr(test_bytes, sizeof(test_bytes));
    EXPECT_FALSE(DecodeInt64(teststr, &n)); // no embedded NULs
    
    // Overflow and underflow
    EXPECT_FALSE(DecodeInt64("-9223372036854775809", nullptr));
    EXPECT_FALSE(DecodeInt64("9223372036854775808", nullptr));
    EXPECT_FALSE(DecodeInt64("-32482348723847471234", nullptr));
    EXPECT_FALSE(DecodeInt64("32482348723847471234", nullptr));
}

TEST(StringEncodingTest, DecodeUint32)
{
    uint32_t n;
    
    // Valid values
    EXPECT_TRUE(DecodeUint32("1234", nullptr));
    EXPECT_TRUE(DecodeUint32("0", &n) && n == 0);
    EXPECT_TRUE(DecodeUint32("1234", &n) && n == 1234);
    EXPECT_TRUE(DecodeUint32("01234", &n) && n == 1234); // no octal
    EXPECT_TRUE(DecodeUint32("2147483647", &n) && n == 2147483647);
    EXPECT_TRUE(DecodeUint32("2147483648", &n) && n == (uint32_t)2147483648);
    EXPECT_TRUE(DecodeUint32("4294967295", &n) && n == (uint32_t)4294967295);
    
    // Invalid values
    EXPECT_FALSE(DecodeUint32("", &n));
    EXPECT_FALSE(DecodeUint32(" 1", &n)); // no padding inside
    EXPECT_FALSE(DecodeUint32(" -1", &n));
    EXPECT_FALSE(DecodeUint32("1 ", &n));
    EXPECT_FALSE(DecodeUint32("1a", &n));
    EXPECT_FALSE(DecodeUint32("aap", &n));
    EXPECT_FALSE(DecodeUint32("0x1", &n)); // no hex
    EXPECT_FALSE(DecodeUint32("0x1", &n)); // no hex
    const char test_bytes[] = {'1', 0, '1'};
    std::string teststr(test_bytes, sizeof(test_bytes));
    EXPECT_FALSE(DecodeUint32(teststr, &n)); // no embedded NULs
    
    // Overflow and underflow
    EXPECT_FALSE(DecodeUint32("-2147483648", &n));
    EXPECT_FALSE(DecodeUint32("4294967296", &n));
    EXPECT_FALSE(DecodeUint32("-1234", &n));
    EXPECT_FALSE(DecodeUint32("-32482348723847471234", nullptr));
    EXPECT_FALSE(DecodeUint32("32482348723847471234", nullptr));
}

TEST(StringEncodingTest, DecodeUint64)
{
    uint64_t n;
    
    // Valid values
    EXPECT_TRUE(DecodeUint64("1234", nullptr));
    EXPECT_TRUE(DecodeUint64("0", &n) && n == 0LL);
    EXPECT_TRUE(DecodeUint64("1234", &n) && n == 1234LL);
    EXPECT_TRUE(DecodeUint64("01234", &n) && n == 1234LL); // no octal
    EXPECT_TRUE(DecodeUint64("2147483647", &n) && n == 2147483647LL);
    EXPECT_TRUE(DecodeUint64("9223372036854775807", &n) && n == 9223372036854775807ULL);
    EXPECT_TRUE(DecodeUint64("9223372036854775808", &n) && n == 9223372036854775808ULL);
    EXPECT_TRUE(DecodeUint64("18446744073709551615", &n) && n == 18446744073709551615ULL);
    
    // Invalid values
    EXPECT_FALSE(DecodeUint64("", &n));
    EXPECT_FALSE(DecodeUint64(" 1", &n)); // no padding inside
    EXPECT_FALSE(DecodeUint64(" -1", &n));
    EXPECT_FALSE(DecodeUint64("1 ", &n));
    EXPECT_FALSE(DecodeUint64("1a", &n));
    EXPECT_FALSE(DecodeUint64("aap", &n));
    EXPECT_FALSE(DecodeUint64("0x1", &n)); // no hex
    const char test_bytes[] = {'1', 0, '1'};
    std::string teststr(test_bytes, sizeof(test_bytes));
    EXPECT_FALSE(DecodeUint64(teststr, &n)); // no embedded NULs
    
    // Overflow and underflow
    EXPECT_FALSE(DecodeUint64("-9223372036854775809", nullptr));
    EXPECT_FALSE(DecodeUint64("18446744073709551616", nullptr));
    EXPECT_FALSE(DecodeUint64("-32482348723847471234", nullptr));
    EXPECT_FALSE(DecodeUint64("-2147483648", &n));
    EXPECT_FALSE(DecodeUint64("-9223372036854775808", &n));
    EXPECT_FALSE(DecodeUint64("-1234", &n));
}

} // namespace unit_test
} // namespace btclit
