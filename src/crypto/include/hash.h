#ifndef BTCLITE_HASH_H
#define BTCLITE_HASH_H

#include <botan/hash.h>
#include <botan/hex.h>
#include <botan/mac.h>
#include <botan/system_rng.h>

#include "arithmetic.h"

using Hash256 = Uint<256>;

void DoubleSha256(const uint8_t in[], size_t length, Hash256 *out);
void DoubleSha256(const std::vector<uint8_t> &in, Hash256 *out);
void DoubleSha256(const std::string &in, Hash256 *out);

/* SipHash-2-4 */
class SipHasher {
public:
	SipHasher()
		: mac(Botan::MessageAuthenticationCode::create_or_throw("SipHash")),
		  key(rng.random_vec(sizeof(uint64_t))) {}
	
	SipHasher& Update(uint64_t in);
	SipHasher& Update(const uint8_t *in, size_t length);
	uint64_t Final();

private:
	std::unique_ptr<Botan::MessageAuthenticationCode> mac;
	Botan::System_RNG rng;
	const Botan::secure_vector<uint8_t> key;	
};

#endif // BTCLITE_Hash256_H
