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
    
    HashOStream();
    
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
