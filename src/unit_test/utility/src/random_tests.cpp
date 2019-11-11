#include <gtest/gtest.h>

#include "random.h"


using namespace btclite::utility::random;

TEST(RandomTest, MethodGetUint64)
{
    ASSERT_EQ(GetUint64(0), 0);
    for (int i = 0; i < 1000; i++) {
        uint64_t rand = GetUint64(10);
        ASSERT_LE(rand, 10);
    }
}

TEST(RandomTest, MethodGetUint256)
{
    Uint256 new_num, old_num;
    old_num = GetUint256();
    for (int i = 0; i < 1000; i++) {
        new_num = GetUint256();
        ASSERT_NE(old_num, new_num);
        old_num = new_num;
    }
}

TEST(FastRandomContextTest, Rand)
{
    // Check that deterministic FastRandomContexts are deterministic
    FastRandomContext ctx1(true);
    FastRandomContext ctx2(true);

    EXPECT_EQ(ctx1.Rand32(), ctx2.Rand32());
    EXPECT_EQ(ctx1.Rand32(), ctx2.Rand32());
    EXPECT_EQ(ctx1.Rand64(), ctx2.Rand64());
    EXPECT_EQ(ctx1.RandBits(3), ctx2.RandBits(3));
    EXPECT_EQ(ctx1.RandBytes(17), ctx2.RandBytes(17));
    EXPECT_EQ(ctx1.Rand256(), ctx2.Rand256());
    EXPECT_EQ(ctx1.RandBits(7), ctx2.RandBits(7));
    EXPECT_EQ(ctx1.RandBytes(128), ctx2.RandBytes(128));
    EXPECT_EQ(ctx1.Rand32(), ctx2.Rand32());
    EXPECT_EQ(ctx1.RandBits(3), ctx2.RandBits(3));
    EXPECT_EQ(ctx1.Rand256(), ctx2.Rand256());
    EXPECT_EQ(ctx1.RandBytes(50), ctx2.RandBytes(50));

    // Check that a nondeterministic ones are not
    FastRandomContext ctx3;
    FastRandomContext ctx4;
    EXPECT_NE(ctx3.Rand64(), ctx4.Rand64()); // extremely unlikely to be equal
    EXPECT_NE(ctx3.Rand256(), ctx4.Rand256());
    EXPECT_NE(ctx3.RandBytes(7), ctx4.RandBytes(7));
}

TEST(FastRandomContextTest, MethodRandbits)
{
    FastRandomContext ctx1;
    FastRandomContext ctx2;
    for (int bits = 0; bits < 63; ++bits) {
        for (int j = 0; j < 1000; ++j) {
            uint64_t rangebits = ctx1.RandBits(bits);
            ASSERT_EQ(rangebits >> bits, 0);
            uint64_t range = ((uint64_t)1) << bits | rangebits;
            uint64_t rand = ctx2.RandRange(range);
            ASSERT_LT(rand, range);
        }
    }
}
