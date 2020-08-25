#ifndef BTCLITE_HASH_H
#define BTCLITE_HASH_H

#include <botan/hash.h>
#include <botan/hex.h>
#include <botan/mac.h>
#include <botan/system_rng.h>

#include "arithmetic.h"
#include "serialize.h"


namespace btclite {
namespace crypto {

const util::Hash256 null_hash
{
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
};

// for std::hash<util::Hash256> in std::unordered_map
template <typename T>
class Hasher {
public:
    size_t operator()(const T& val) const
    {
        return util::FromLittleEndian<uint64_t>(val.data());
    }
};


// A hasher class for SHA-256.
class SHA256 {
public:
    static const size_t OUTPUT_SIZE = 32;

    SHA256()
        : bytes_(0)
    {
        Initialize();
    }
    
    SHA256& Write(const uint8_t *data, size_t len);
    void Finalize(uint8_t out[OUTPUT_SIZE]);
    
    SHA256& Reset()
    {
        bytes_ = 0;
        Initialize();
        return *this;
    }
    
private:
    uint32_t s_[8];
    uint8_t buf_[64];
    uint64_t bytes_;
    
    // Initialize SHA-256 state.
    void Initialize()
    {
        s_[0] = 0x6a09e667ul;
        s_[1] = 0xbb67ae85ul;
        s_[2] = 0x3c6ef372ul;
        s_[3] = 0xa54ff53aul;
        s_[4] = 0x510e527ful;
        s_[5] = 0x9b05688cul;
        s_[6] = 0x1f83d9abul;
        s_[7] = 0x5be0cd19ul;
    }
    
    // Perform a number of SHA-256 transformations, processing 64-byte chunks.
    void Transform(const uint8_t *chunk, size_t blocks);
    
    uint32_t inline Ch(uint32_t x, uint32_t y, uint32_t z)
    {
        return z ^ (x & (y ^ z)); 
    }

    uint32_t inline Maj(uint32_t x, uint32_t y, uint32_t z) 
    { 
        return (x & y) | (z & (x | y)); 
    }

    uint32_t inline Sigma0(uint32_t x) 
    { 
        return (x >> 2 | x << 30) ^ (x >> 13 | x << 19) ^ (x >> 22 | x << 10); 
    }

    uint32_t inline Sigma1(uint32_t x)
    { 
        return (x >> 6 | x << 26) ^ (x >> 11 | x << 21) ^ (x >> 25 | x << 7); 
    }

    uint32_t inline sigma0(uint32_t x)
    { 
        return (x >> 7 | x << 25) ^ (x >> 18 | x << 14) ^ (x >> 3); 
    }

    uint32_t inline sigma1(uint32_t x) 
    { 
        return (x >> 17 | x << 15) ^ (x >> 19 | x << 13) ^ (x >> 10); 
    }
    
    // One round of SHA-256.
    void inline Round(uint32_t a, uint32_t b, uint32_t c, uint32_t& d, uint32_t e, uint32_t f, uint32_t g, uint32_t& h, uint32_t k, uint32_t w)
    {
        uint32_t t1 = h + Sigma1(e) + Ch(e, f, g) + k + w;
        uint32_t t2 = Sigma0(a) + Maj(a, b, c);
        d += t1;
        h = t1 + t2;
    }
};


namespace hashfuncs {

util::Hash256 Sha256(const uint8_t in[], size_t length);
util::Hash256 Sha256(const std::vector<uint8_t>& in);

util::Hash256 DoubleSha256(const uint8_t in[], size_t length);
util::Hash256 DoubleSha256(const std::vector<uint8_t>& in);
util::Hash256 DoubleSha256(const std::string& in);

} // namespace hashfuncs


// A writer stream (for serialization) that computes a 256-bit hash.
class HashOStream {
public:
    using Container = std::vector<uint8_t>;
    using ByteSinkType = util::ByteSink<Container>;
    
    HashOStream()
        : vec_(), byte_sink_(vec_) {}
    
    //-------------------------------------------------------------------------
    util::Hash256 Sha256();    
    util::Hash256 DoubleSha256();
    
    //-------------------------------------------------------------------------
    template <typename T>
    HashOStream& operator<<(const T& obj)
    {
        util::Serializer<ByteSinkType> serializer(byte_sink_);
        serializer.SerialWrite(obj);
        return *this;
    }
    
    //-------------------------------------------------------------------------
    size_t Size() const;    
    const Container& vec() const;    
    void Clear();
    
private:
    Container vec_;
    ByteSinkType byte_sink_;
};


// SipHash-2-4
class SipHasher {
public:
    // Construct a SipHash calculator initialized with 128-bit key.
    SipHasher();
    SipHasher(uint64_t k0, uint64_t k1);
    
    /* Hash a 64-bit integer worth of data
    *  It is treated as if this was the little-endian interpretation of 8 bytes_.
    *  This function can only be used when a multiple of 8 bytes_ have been written so far.
    */
    SipHasher& Update(uint64_t in);
    // Hash arbitrary bytes_.
    SipHasher& Update(const uint8_t *in, size_t length);
    
    // Compute the 64-bit SipHash-2-4 of the data written so far. The object remains untouched.
    uint64_t Final();

private:
    std::unique_ptr<Botan::MessageAuthenticationCode> mac_;
    Botan::System_RNG rng_;
    Botan::secure_vector<uint8_t> key_;
    bool is_set_key_;
};


// support hashable for any class with Serialize methord
template <typename T>
util::Hash256 GetHash(const T& obj)
{
    HashOStream hs;
    hs << obj;
    return hs.Sha256();
}

template <typename T>
util::Hash256 GetDoubleHash(const T& obj)
{
    HashOStream hs;
    hs << obj;
    return hs.DoubleSha256();
}

} // namespace crypto
} // namespace btclite

#endif // BTCLITE_Hash256_H
