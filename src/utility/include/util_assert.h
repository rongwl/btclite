#ifndef BTCLITE_UTIL_ASSERT_H
#define BTCLITE_UTIL_ASSERT_H

#include <cassert>
#include <type_traits>


namespace btclite {
namespace util {

#define ASSERT_UNSIGNED(T) static_assert(std::is_unsigned<T>::value, \
                                         "This function requires unsigned type")
#define ASSERT_ARITHMETIC(T) static_assert(std::is_arithmetic<T>::value, \
                                           "This function requires arithmetic type")
#define ASSERT_INTEGRAL(T) static_assert(std::is_integral<T>::value, \
                                         "This function requires integral type")
#define ASSERT_SIGNED(T) static_assert(std::is_signed<T>::value, \
                                         "This function requires signed type")

#define ASSERT_NULL(OUT) assert(OUT != nullptr)

} // namespace util
} // namespace btclite

#endif // BTCLITE_UTIL_ASSERT_H
