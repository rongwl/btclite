#include "random.h"

#include <botan/system_rng.h>
#include <random>


namespace btclite {
namespace util {

uint64_t RandUint64(uint64_t max)
{
    if (max == 0)
        return 0;

    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 rng(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<uint64_t> dis(0, max); 
    
    return dis(rng);
}

util::Hash256 RandHash256() 
{
    Botan::System_RNG rng;
    util::Hash256 hash;
    rng.randomize(hash.data(), hash.size());
    
    return hash;
}

FastRandomContext::FastRandomContext(const util::Hash256& seed)
    :  rng_(Botan::secure_vector<uint8_t>(seed.begin(), seed.end())), 
       requires_seed_(false), bytebuf_size_(0), bitbuf_size_(0) 
{
}

FastRandomContext::FastRandomContext(bool deterministic)
    : rng_(), requires_seed_(!deterministic), bytebuf_size_(0), bitbuf_size_(0)
{
    if (!deterministic) {
        return;
    }
    util::Hash256 seed;
    rng_.add_entropy(seed.data(), seed.size());
}

uint64_t FastRandomContext::RandBits(int bits) {
    if (bits == 0) {
        return 0;
    } else if (bits > 32) {
        return Rand64() >> (64 - bits);
    } else {
        if (bitbuf_size_ < bits)
            FillBitBuffer();
        uint64_t ret = bitbuf_ & (~(uint64_t)0 >> (64 - bits));
        bitbuf_ >>= bits;
        bitbuf_size_ -= bits;

        return ret;
    }
}

uint64_t FastRandomContext::RandRange(uint64_t range)
{
    --range;
    int bits = CountBits(range);

    while (true) {
        uint64_t ret = RandBits(bits);
        if (ret <= range)
            return ret;
    }
}

void FastRandomContext::RandomSeed()
{
    util::Hash256 seed = RandHash256();
    rng_.add_entropy(seed.data(), seed.size());
    requires_seed_ = false;
}

std::vector<unsigned char> FastRandomContext::RandBytes(size_t len)
{
    std::vector<unsigned char> ret(len);
    if (len > 0) {
        rng_.randomize(ret.data(), len);
    }
    
    return ret;
}

uint32_t FastRandomContext::Rand32()
{
    return RandBits(32);
}

uint64_t FastRandomContext::Rand64()
{
    if (bytebuf_size_ < 8)
        FillByteBuffer();
    uint64_t ret = FromLittleEndian<uint64_t>(bytebuf_ + 64 - bytebuf_size_);
    bytebuf_size_ -= 8;
    return ret;
}

util::Hash256 FastRandomContext::Rand256()
{
    if (bytebuf_size_ < 32) {
        FillByteBuffer();
    }
    util::Hash256 ret;
    memcpy(ret.begin(), bytebuf_ + 64 - bytebuf_size_, 32);
    bytebuf_size_ -= 32;
    
    return ret;
}

bool FastRandomContext::RandBool()
{
    return RandBits(1);
}

bool FastRandomContext::requires_seed() const
{
    return requires_seed_;
}

int FastRandomContext::bytebuf_size() const
{
    return bytebuf_size_;
}

uint64_t FastRandomContext::bitbuf() const
{
    return bitbuf_;
}

int FastRandomContext::bitbuf_size() const
{
    return bitbuf_size_;
}

void FastRandomContext::FillByteBuffer()
{
    if (requires_seed_) {
        RandomSeed();
    }

    rng_.randomize(bytebuf_, sizeof(bytebuf_));
    bytebuf_size_ = sizeof(bytebuf_);
}

uint64_t FastRandomContext::CountBits(uint64_t x)
{
    int ret = 0;
    while (x) {
        x >>= 1;
        ++ret;
    }
    
    return ret;
}

void FastRandomContext::FillBitBuffer()
{
    bitbuf_ = Rand64();
    bitbuf_size_ = 64;
}

} // namespace util
} // namespace btclite
