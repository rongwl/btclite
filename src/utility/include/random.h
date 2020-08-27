#ifndef BTCLITE_RANDOM_H
#define BTCLITE_RANDOM_H

#include <botan/chacha_rng.h>
#include <cstdint>

#include "arithmetic.h"
#include "util_endian.h"


namespace btclite {
namespace util {

uint64_t RandUint64(uint64_t max = std::numeric_limits<uint64_t>::max());
util::Hash256 RandHash256();


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
    explicit FastRandomContext(const util::Hash256& seed);

    //-------------------------------------------------------------------------
    /** Generate a random (bits)-bit integer. */
    uint64_t RandBits(int bits);

    /** Generate a random integer in the range [0..range). */
    uint64_t RandRange(uint64_t range);

    /** Generate random bytes. */
    std::vector<unsigned char> RandBytes(size_t len);

    /** Generate a random 32-bit integer. */
    uint32_t Rand32();
    
    /** Generate a random 64-bit integer. */
    uint64_t Rand64();

    /** generate a random uint256. */
    util::Hash256 Rand256();

    /** Generate a random boolean. */
    bool RandBool();

    //-------------------------------------------------------------------------
    bool requires_seed() const;    
    int bytebuf_size() const;
    uint64_t bitbuf() const;    
    int bitbuf_size() const;
    
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
    void FillBitBuffer();
};

} // namespace util
} // namespace btclite


#endif // BTCLITE_RANDOM_H
