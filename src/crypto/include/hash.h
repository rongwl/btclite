#ifndef BTCLITE_HASH_H
#define BTCLITE_HASH_H

#include <botan/hash.h>
#include <botan/hex.h>
#include <botan/mac.h>
#include <botan/system_rng.h>

#include "arithmetic.h"

using Hash256 = Blob<256>;


void DoubleSha256(const uint8_t in[], size_t length, Hash256 *out);
void DoubleSha256(const std::vector<uint8_t> &in, Hash256 *out);
void DoubleSha256(const std::string &in, Hash256 *out);


// SipHash-2-4
class SipHasher {
public:
    // Construct a SipHash calculator initialized with 128-bit key.
    SipHasher()
        : mac_(Botan::MessageAuthenticationCode::create_or_throw("SipHash")),
          rng_(), key_(rng_.random_vec(16)), is_set_key_(false) {}
    explicit SipHasher(const Uint128& key)
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

#endif // BTCLITE_Hash256_H
