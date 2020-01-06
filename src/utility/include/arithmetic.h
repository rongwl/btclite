#ifndef BTCLITE_ARITHMETIC_H
#define BTCLITE_ARITHMETIC_H

#include <boost/multiprecision/cpp_int.hpp>
#include <endian.h>
#include <limits>

#include "Assert.h"
#include "blob.h"


namespace btclite {
namespace util {

using int128_t = boost::multiprecision::int128_t;
using uint128_t = boost::multiprecision::uint128_t;
using int256_t = boost::multiprecision::int256_t;
using uint256_t = boost::multiprecision::uint256_t;

class Uint128 : public Blob<128> {
public:
    using Blob<128>::Blob;

    Uint128(uint64_t low, uint64_t high) 
    { 
        std::memcpy(this->begin(), &low, sizeof(uint64_t));
        std::memcpy(this->begin()+sizeof(uint64_t), &high, sizeof(uint64_t));
    }
    
    //-------------------------------------------------------------------------
    uint64_t GetLow64() const
    {
        const uint64_t *x = reinterpret_cast<const uint64_t*>(this->data());
        return *x;
    }
    
    uint32_t GetLow32() const
    {
        const uint32_t *x = reinterpret_cast<const uint32_t*>(this->data());
        return *x;
    }
    
    //-------------------------------------------------------------------------
    friend bool operator==(const Uint128& a, const Uint128& b)
    {
        return (a.Compare(b) == 0);
    }
    
    friend bool operator!=(const Uint128& a, const Uint128& b)
    {
        return !(a == b);
    }
    
    friend bool operator<(const Uint128& a, const Uint128& b)
    {
        return (a.Compare(b) < 0);
    }
    
    friend bool operator>(const Uint128& a, const Uint128& b)
    {
        return (a.Compare(b) > 0);
    }
};

class Uint256 : public Blob<256> {
public:
    using Blob<256>::Blob;
    
    Uint256(const Uint128& low, const Uint128& high)
    {
        std::memcpy(this->begin(), low.begin(), low.size());
        std::memcpy(this->begin()+low.size(), high.begin(), high.size());
    }   
    
    //-------------------------------------------------------------------------
    uint64_t GetLow64() const
    {
        const uint64_t *x = reinterpret_cast<const uint64_t*>(this->data());
        return *x;
    }
    
    uint32_t GetLow32() const
    {
        const uint32_t *x = reinterpret_cast<const uint32_t*>(this->data());
        return *x;
    }
    
    //-------------------------------------------------------------------------
    friend bool operator==(const Uint256& a, const Uint256& b)
    {
        return (a.Compare(b) == 0);
    }
    
    friend bool operator!=(const Uint256& a, const Uint256& b)
    {
        return !(a == b);
    }
    
    friend bool operator<(const Uint256& a, const Uint256& b)
    {
        return (a.Compare(b) < 0);
    }
    
    friend bool operator>(const Uint256& a, const Uint256& b)
    {
        return (a.Compare(b) > 0);
    }
};


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
