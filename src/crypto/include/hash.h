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

using Hash256 = util::Uint256;

// for std::hash<util::Uint256/Uint128> in std::unordered_map
template <typename T>
class Hasher {
public:
    size_t operator()(const T& val) const
    {
        return val.GetLow64();
    }
};


void Sha256(const uint8_t in[], size_t length, Hash256 *out);
Hash256 Sha256(const uint8_t in[], size_t length);
void Sha256(const std::vector<uint8_t>& in, Hash256 *out);
Hash256 Sha256(const std::vector<uint8_t>& in);

void DoubleSha256(const uint8_t in[], size_t length, Hash256 *out);
Hash256 DoubleSha256(const uint8_t in[], size_t length);
void DoubleSha256(const std::vector<uint8_t> &in, Hash256 *out);
Hash256 DoubleSha256(const std::vector<uint8_t> &in);
void DoubleSha256(const std::string &in, Hash256 *out);
Hash256 DoubleSha256(const std::string &in);


// A writer stream (for serialization) that computes a 256-bit hash.
class HashOStream {
public:
    using Container = std::vector<uint8_t>;
    using ByteSinkType = util::ByteSink<Container>;
    
    HashOStream()
        : vec_(), byte_sink_(vec_) {}
    
    void Sha256(Hash256 *out) const; 
    Hash256 Sha256() const;
    void DoubleSha256(Hash256 *out) const;
    Hash256 DoubleSha256() const;
    
    template <typename T>
    HashOStream& operator<<(const T& obj)
    {
        util::Serializer<ByteSinkType> serializer(byte_sink_);
        serializer.SerialWrite(obj);
        return *this;
    }
    
    size_t Size() const
    {
        return vec_.size();
    }
    
    const Container& vec() const
    {
        return vec_;
    }
    
    void Clear()
    {
        vec_.clear();
    }
    
private:
    Container vec_;
    ByteSinkType byte_sink_;
};


// SipHash-2-4
class SipHasher {
public:
    // Construct a SipHash calculator initialized with 128-bit key.
    SipHasher()
        : mac_(Botan::MessageAuthenticationCode::create_or_throw("SipHash")),
          rng_(), key_(rng_.random_vec(16)), is_set_key_(false) {}
    explicit SipHasher(const util::Uint128& key)
        : mac_(Botan::MessageAuthenticationCode::create_or_throw("SipHash")),
          rng_(), key_(key.begin(), key.end()), is_set_key_(false) {}

    
    /* Hash a 64-bit integer worth of data
    *  It is treated as if this was the little-endian interpretation of 8 bytes.
    *  This function can only be used when a multiple of 8 bytes have been written so far.
    */
    SipHasher& Update(uint64_t in);
    // Hash arbitrary bytes.
    SipHasher& Update(const uint8_t *in, size_t length);
    
    // Compute the 64-bit SipHash-2-4 of the data written so far. The object remains untouched.
    uint64_t Final();

private:
    std::unique_ptr<Botan::MessageAuthenticationCode> mac_;
    Botan::System_RNG rng_;
    const Botan::secure_vector<uint8_t> key_;
    bool is_set_key_;
};

// mixin GetHash() for any class with Serialize methord
template <typename Base>
struct Hashable : public Base {
    using Base::Base;
    
    Hash256 GetHash() const;
};

template <typename Base>
Hash256 Hashable<Base>::GetHash() const
{
    std::vector<uint8_t> vec;
    Hash256 hash;
    util::ByteSink<std::vector<uint8_t> > byte_sink(vec);
    
    Base::Serialize(byte_sink);
    crypto::Sha256(vec, &hash);
    
    return hash;
}

} // namespace crypto
} // namespace btclite

#endif // BTCLITE_Hash256_H
