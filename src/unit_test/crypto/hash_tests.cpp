#include <gtest/gtest.h>

#include "hash.h"
#include "transaction.h"


using namespace btclite::crypto::hash;

TEST(HashWStreamTest, Sha256)
{
    Transaction tx;
    HashWStream hs;
    Hash256 hash1;
    Hash256 hash2({0x96,0xee,0xff,0x56,0x3b,0x31,0x35,0xe3,
                   0xf7,0x79,0x64,0xe8,0xc0,0x62,0x32,0x8f,
                   0xd2,0x07,0xc8,0xbc,0x9e,0x75,0x4f,0xc4,
                   0x23,0xab,0xaf,0x83,0xeb,0x3f,0x14,0x90});
    
    tx.set_version(1);    
    hs << tx;
    ASSERT_TRUE(hash1.IsNull());
    hs.Sha256(&hash1);
    EXPECT_EQ(hash1, hash2);
}

TEST(HashWStreamTest, DoubleSha256)
{
    Transaction tx;
    HashWStream hs;
    Hash256 hash1;
    Hash256 hash2({0x43,0xec,0x7a,0x57,0x9f,0x55,0x61,0xa4,
                   0x2a,0x7e,0x96,0x37,0xad,0x41,0x56,0x67,
                   0x27,0x35,0xa6,0x58,0xbe,0x27,0x52,0x18,
                   0x18,0x01,0xf7,0x23,0xba,0x33,0x16,0xd2});
    
    tx.set_version(1);    
    hs << tx;
    ASSERT_TRUE(hash1.IsNull());
    hs.DoubleSha256(&hash1);
    EXPECT_EQ(hash1, hash2);
}

TEST(HashTest, Sha256)
{
    Transaction tx;
    HashWStream hs;
    Hash256 hash1;
    Hash256 hash2({0x96,0xee,0xff,0x56,0x3b,0x31,0x35,0xe3,
                   0xf7,0x79,0x64,0xe8,0xc0,0x62,0x32,0x8f,
                   0xd2,0x07,0xc8,0xbc,0x9e,0x75,0x4f,0xc4,
                  0x23,0xab,0xaf,0x83,0xeb,0x3f,0x14,0x90});
    
    tx.set_version(1);    
    hs << tx;
    Sha256(hs.vec(), &hash1);
    EXPECT_EQ(hash1, hash2);
    
    hash1.Clear();
    ASSERT_TRUE(hash1.IsNull());
    Sha256(hs.vec().data(), hs.vec().size(), &hash1);
    EXPECT_EQ(hash1, hash2);
}

TEST(HashTest, DoubleSha256)
{
    Transaction tx;
    HashWStream hs;
    Hash256 hash1;
    Hash256 hash2({0x43,0xec,0x7a,0x57,0x9f,0x55,0x61,0xa4,
                   0x2a,0x7e,0x96,0x37,0xad,0x41,0x56,0x67,
                   0x27,0x35,0xa6,0x58,0xbe,0x27,0x52,0x18,
                  0x18,0x01,0xf7,0x23,0xba,0x33,0x16,0xd2});
    
    tx.set_version(1);    
    hs << tx;
    ASSERT_TRUE(hash1.IsNull());
    DoubleSha256(hs.vec(), &hash1);
    
    hash1.Clear();
    ASSERT_TRUE(hash1.IsNull());
    DoubleSha256(hs.vec().data(), hs.vec().size(), &hash1);
    EXPECT_EQ(hash1, hash2);
}


TEST(SipHasherTest, Constructor1)
{
    SipHasher sip_hasher;
    
    uint64_t tag1 = sip_hasher.Update(0x1122334455667788).Final();
    uint64_t tag2 = sip_hasher.Update(0x1122334455667788).Final();
    ASSERT_EQ(tag1, tag2);
    tag2 = sip_hasher.Update(0x1122334455667788+1).Final();
    EXPECT_NE(tag1, tag2);
}

TEST(SipHasherTest, Constructor2)
{
    Uint128 key(0x12345678, 0x12345678);
    SipHasher sip_hasher(key);
    uint64_t tag = sip_hasher.Update(0x1122334455667788).Final();
    ASSERT_EQ(tag, 0xb0bc17a3d48ce99a);
}
