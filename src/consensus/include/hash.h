#ifndef BTCLITE_HASH_H
#define BTCLITE_HASH_H

#include <botan/hash.h>

#include "arithmetic.h"

using Hash256 = Uint<256>;

void DoubleSha256(const uint8_t in[], std::size_t length, Hash256 *out);
void DoubleSha256(const std::vector<uint8_t> &in, Hash256 *out);
void DoubleSha256(const std::string &in, Hash256 *out);

#endif // BTCLITE_Hash256_H
