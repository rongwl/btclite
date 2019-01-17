#ifndef BTCLITE_ASSERT_H
#define BTCLITE_ASSERT_H

#include <cassert>
#include <type_traits>

#define ASSERT_UNSIGNED(T) static_assert(std::is_unsigned<T>::value, \
                                         "This function requires unsigned type")

#define ASSERT_NULL(OUT) assert(OUT != nullptr)

#endif // BTCLITE_ASSERT_H
