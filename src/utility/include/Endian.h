#ifndef BTCLITE_ENDIAN_H
#define BTCLITE_ENDIAN_H

#include <cstddef>
#include "Assert.h"
#include "blob.h"


namespace btclite {
namespace util {

template <typename T>
void ToBigEndian(T in, Bytes<sizeof(T)> *out)
{
    ASSERT_NULL(out);
    for (auto it = out->rbegin(); it != out->rend(); ++it) {
        *it = static_cast<uint8_t>(in);
        in >>= 8;
    }
}

template <typename T>
void ToLittleEndian(T in, Bytes<sizeof(T)> *out)
{
    ASSERT_NULL(out);
    for (auto it = out->begin(); it != out->end(); ++it) {
        *it = static_cast<uint8_t>(in);
        in >>= 8;
    }
}

template <typename T, typename Iterator>
T FromBigEndian(Iterator begin)
{
    //ASSERT_UNSIGNED(T);    
    T out = 0;
    auto end = begin + sizeof(T);
    while (begin != end)
        out = (out << 8) | static_cast<T>(*begin++);
    
    return out;
}

template <typename T, typename Iterator>
void FromBigEndian(Iterator begin, Iterator end, T *out)
{
    //ASSERT_UNSIGNED(T);
    ASSERT_NULL(out);
    *out = 0;
    while (begin != end)
        *out = (*out << 8) | static_cast<T>(*begin++);
}

template <typename T, typename Iterator>
T FromLittleEndian(Iterator begin)
{
    //ASSERT_UNSIGNED(T);    
    T out = 0;
    size_t i = 0;
    while (i < sizeof(T)) 
        out |= static_cast<T>(*begin++) << (8 * i++);
    
    return out;
}

template <typename T, typename Iterator>
void FromLittleEndian(Iterator begin, Iterator end, T *out)
{
    //ASSERT_UNSIGNED(T);
    ASSERT_NULL(out);
    size_t i = 0;
    *out = 0;
    while (i < sizeof(T) && begin != end) 
        *out |= (static_cast<T>(*begin++) << (8 * i++));
}

} // namespace util
} // namespace btclite

#endif // BTCLITE_ENDIAN_H
