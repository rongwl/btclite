#include <gtest/gtest.h>

#include "random.h"

TEST(RandomTest, MethordGet)
{
    EXPECT_EQ(Random::Get(0), 0);
    uint64_t rand = Random::Get(10);
    EXPECT_TRUE(rand >= 0 && rand <= 10);
}
