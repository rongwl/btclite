#ifndef BTCLITE_RANDOM_H
#define BTCLITE_RANDOM_H

#include <botan/chacha_rng.h>
#include <cstdint>

#include "arithmetic.h"
#include "Endian.h"


class Random {
public:
    static uint64_t GetUint64(uint64_t max);
    static Uint256 GetUint256();
};

/* 
 * Fast randomness source. Merge from bitcoin core code.
 * This is seeded once with secure random data, but
 * is completely deterministic and insecure after that.
 * This class is not thread-safe.
 */
class FastRandomContext {
public:
    explicit FastRandomContext(bool deterministic = false);

    /** Initialize with explicit seed (only for testing) */
    explicit FastRandomContext(const Uint256& seed)
        :  rng_(Botan::secure_vector<uint8_t>(seed.begin(), seed.end())), 
           requires_seed_(false), bytebuf_size_(0), bitbuf_size_(0) {}

    //-------------------------------------------------------------------------
    /** Generate a random (bits)-bit integer. */
    uint64_t RandBits(int bits);

    /** Generate a random integer in the range [0..range). */
    uint64_t RandRange(uint64_t range);

    /** Generate random bytes. */
    std::vector<unsigned char> RandBytes(size_t len);

    /** Generate a random 32-bit integer. */
    uint32_t Rand32()
    {
        return RandBits(32);
    }
    
    /** Generate a random 64-bit integer. */
    uint64_t Rand64()
    {
        if (bytebuf_size_ < 8)
            FillByteBuffer();
        uint64_t ret = FromLittleEndian<uint64_t>(bytebuf_ + 64 - bytebuf_size_);
        bytebuf_size_ -= 8;
        return ret;
    }

    /** generate a random uint256. */
    Uint256 Rand256();

    /** Generate a random boolean. */
    bool RandBool()
    {
        return RandBits(1);
    }

    bool requires_seed() const
    {
        return requires_seed_;
    }
    
    int bytebuf_size() const
    {
        return bytebuf_size_;
    }
    
    uint64_t bitbuf() const
    {
        return bitbuf_;
    }
    
    int bitbuf_size() const
    {
        return bitbuf_size_;
    }
private:
    Botan::ChaCha_RNG rng_;
    bool requires_seed_;
    unsigned char bytebuf_[64];
    int bytebuf_size_;
    uint64_t bitbuf_;
    int bitbuf_size_;

    void RandomSeed();
    void FillByteBuffer();    
    uint64_t CountBits(uint64_t x);
    void FillBitBuffer()
    {
        bitbuf_ = Rand64();
        bitbuf_size_ = 64;
    }
};


#endif // BTCLITE_RANDOM_H
