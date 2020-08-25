#ifndef BTCLITE_ARITHMETIC_H
#define BTCLITE_ARITHMETIC_H

#include <boost/multiprecision/cpp_int.hpp>
#include <endian.h>
#include <limits>

#include "blob.h"
#include "util_assert.h"


namespace btclite {
namespace util {

using int128_t = boost::multiprecision::int128_t;
using uint128_t = boost::multiprecision::uint128_t;
using int256_t = boost::multiprecision::int256_t;
using uint256_t = boost::multiprecision::uint256_t;

using Hash256 = Bytes<32>;

Hash256 StrToHash256(const std::string& s);


template <typename T>
class Integral {
public:
    T value() const
    {
        return value_;
    }
    void set_value(const T& v)
    {
        value_ = v;
    }
    
    virtual bool operator==(const T& b)
    {
        return (value_ == b);
    }
    virtual bool operator!=(const T& b)
    {
        return (value_ != b);
    }
    virtual bool operator<(const T& b)
    {
        return (value_ < b);
    }
    virtual bool operator<=(const T& b)
    {
        return (value_ <= b);
    }
    virtual bool operator>(const T& b)
    {
        return (value_ > b);
    }
    virtual bool operator>=(const T& b)
    {
        return (value_ >= b);
    }
    
    virtual bool operator==(const Integral& b)
    {
        return (value_ == b.value_);
    }
    virtual bool operator!=(const Integral& b)
    {
        return !(*this == b);
    }
    virtual bool operator<(const Integral& b)
    {
        return (value_ < b.value_);
    }
    virtual bool operator<=(const Integral& b)
    {
        return (value_ <= b.value_);
    }
    virtual bool operator>(const Integral& b)
    {
        return (value_ > b.value_);
    }
    virtual bool operator>=(const Integral& b)
    {
        return (value_ >= b.value_);
    }
    
    virtual Integral operator+(const T& b)
    {
        assert((b > 0 && value_ + b <= std::numeric_limits<T>::max()) ||
               (b < 0 && value_ + b >= std::numeric_limits<T>::min()));
        return Integral(value_+b);
    }
    virtual Integral& operator+=(const T& b)
    {
        assert((b > 0 && value_ + b <= std::numeric_limits<T>::max()) ||
               (b < 0 && value_ + b >= std::numeric_limits<T>::min()));
        value_ += b;
        return *this;
    }
    virtual Integral operator-(const T& b)
    {
        assert((b > 0 && value_ - b >= std::numeric_limits<T>::min()) ||
               (b < 0 && value_ - b <= std::numeric_limits<T>::max()));
        return Integral(value_-b);
    }
    virtual Integral& operator-=(const T& b)
    {
        assert((b > 0 && value_ - b >= std::numeric_limits<T>::min()) ||
               (b < 0 && value_ - b <= std::numeric_limits<T>::max()));
        value_ -= b;
        return *this;
    }
    
    virtual Integral operator+(const Integral& b)
    {
        return operator+(b.value_);
    }
    virtual Integral operator-(const Integral& b)
    {
        return operator-(b.value_);
    }
    virtual Integral& operator+=(const Integral& b)
    {
        return operator+=(b.value_);
    }
    virtual Integral& operator-=(const Integral& b)
    {
        return operator-=(b.value_);
    }
    
    virtual std::enable_if_t<std::is_integral<T>::value, Integral> operator&(const T& b)
    {
        return Integral(value_ & b);
    }
    virtual std::enable_if_t<std::is_integral<T>::value, Integral&> operator&=(const T& b)
    {
        value_ &= b;
        return *this;
    }
    virtual std::enable_if_t<std::is_integral<T>::value, Integral> operator|(const T& b)
    {
        return Integral(value_ & b);
    }
    virtual std::enable_if_t<std::is_integral<T>::value, Integral&> operator|=(const T& b)
    {
        value_ |= b;
        return *this;
    }
    
    virtual std::enable_if_t<std::is_integral<T>::value, Integral> operator&(const Integral& b)
    {
        return Integral(value_ & b.value_);
    }
    virtual std::enable_if_t<std::is_integral<T>::value, Integral&> operator&=(const Integral& b)
    {
        value_ &= b.value_;
        return *this;
    }
    virtual std::enable_if_t<std::is_integral<T>::value, Integral> operator|(const Integral& b)
    {
        return Integral(value_ | b.value_);
    }
    virtual std::enable_if_t<std::is_integral<T>::value, Integral&> operator|=(const Integral& b)
    {
        value_ |= b.value_;
        return *this;
    }
    
    virtual std::enable_if_t<std::is_signed<T>::value, Integral> operator-()
    {
        return Integral(-value_);
    }
    
protected:
    Integral()
    {
        ASSERT_ARITHMETIC(T);
    }
    
private:
    Integral(const T& v)
        : value_(v) {}
    
    Integral& operator=(const Integral& v)
    {
        value_ = v.value_;
        return *this;
    }
    
    T value_;
};

} // namespace util
} // namespace btclite


#endif // BTCLITE_ARITHMETIC_H
