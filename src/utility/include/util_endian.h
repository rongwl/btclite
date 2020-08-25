#ifndef BTCLITE_UTIL_ENDIAN_H
#define BTCLITE_UTIL_ENDIAN_H

#include <cstddef>
#include <cstdint>

#include "util_assert.h"


namespace btclite {
namespace util {

template <typename T>
void ToBigEndian(T in, uint8_t *out_low)
{
    auto out_high = out_low + sizeof(T) - 1;
    while (out_high >= out_low) {
        *out_high = static_cast<uint8_t>(in);
        in >>= 8;
        out_high--;
    }
}

template <typename T>
void ToLittleEndian(T in, uint8_t *out_low)
{
    auto out_high = out_low + sizeof(T) - 1;
    while (out_low <= out_high) {
        *out_low = static_cast<uint8_t>(in);
        in >>= 8;
        out_low++;
    }
}

template <typename T>
T FromBigEndian(const uint8_t *low)
{
    //ASSERT_UNSIGNED(T);    
    T out = 0;
    auto high = low + sizeof(T) - 1;
    while (low <= high) {
        out = (out << 8) | static_cast<T>(*low);
        low++;
    }
    
    return out;
}


template <typename T>
T FromLittleEndian(const uint8_t *low)
{
    //ASSERT_UNSIGNED(T);    
    T out = 0;
    size_t i = 0;
    while (i < sizeof(T)) {
        out |= static_cast<T>(*low) << (8 * i);
        low++;
        i++;
    }
    
    return out;
}

} // namespace util
} // namespace btclite

#endif // BTCLITE_UTIL_ENDIAN_H
