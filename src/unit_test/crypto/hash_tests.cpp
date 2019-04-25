#include <gtest/gtest.h>

#include "hash.h"

TEST(SipHasherTest, Constructor1)
{
    SipHasher sip_hasher;
    
    uint64_t tag1 = sip_hasher.Update(0x1122334455667788).Final();
    uint64_t tag2 = sip_hasher.Update(0x1122334455667788).Final();
    EXPECT_EQ(tag1, tag2);
    tag2 = sip_hasher.Update(0x1122334455667788+1).Final();
    EXPECT_NE(tag1, tag2);
}

TEST(SipHasherTest, Constructor2)
{
    Uint128 key(0x12345678, 0x12345678);
    SipHasher sip_hasher(key);
    uint64_t tag = sip_hasher.Update(0x1122334455667788).Final();
    EXPECT_EQ(tag, 0xb0bc17a3d48ce99a);
}
