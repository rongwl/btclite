#ifndef BTCLITE_ENDIAN_H
#define BTCLITE_ENDIAN_H

#include <cstddef>
#include "Assert.h"

template <std::size_t size>
using Bytes = std::array<std::byte, size>;

template <typename T>
void ToBigEndian(T in, Bytes<sizeof(T)> *out)
{
	ASSERT_UNSIGNED(T);
	ASSERT_NULL(out);
	for (auto it = out->rbegin(); it != out->rend(); ++it) {
		*it = static_cast<std::byte>(in);
		in >>= 8;
	}
}

template <typename T>
void ToLittleEndian(T in, Bytes<sizeof(T)> *out)
{
	ASSERT_UNSIGNED(T);
	ASSERT_NULL(out);
	for (auto it = out->begin(); it != out->end(); ++it) {
		*it = static_cast<std::byte>(in);
		in >>= 8;
	}
}

template <typename T>
void FromBigEndian(const Bytes<sizeof(T)>& in, T *out)
{
	ASSERT_UNSIGNED(T);
	ASSERT_NULL(out);
	for (auto it = in.begin(); it != in.end(); ++it) {
		*out = (*out << 8) | static_cast<unsigned int>(*it);
	}
}

template <typename T>
void FromLittleEndian(const Bytes<sizeof(T)>& in, T *out)
{
	ASSERT_UNSIGNED(T);
	ASSERT_NULL(out);
	for (auto it = in.rbegin(); it != in.rend(); ++it)
		*out = (*out << 8) | static_cast<unsigned int>(*it);
}

#endif // BTCLITE_ENDIAN_H
