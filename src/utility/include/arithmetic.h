#ifndef BTCLITE_ARITHMETIC_H
#define BTCLITE_ARITHMETIC_H

#include <limits>

#include "Assert.h"
#include "blob.h"

template <uint32_t nBITS>
class Uint : public Blob<nBITS> {
public:
	Uint()
	{
		this->SetNull();
	}
	
	// Operation
	//-------------------------------------------------------------------------
	friend bool operator==(const Uint& a, const Uint& b)
	{
		return (a.Compare(b) == 0);
	}
	friend bool operator!=(const Uint& a, const Uint& b)
	{
		return !(a == b);
	}
	friend bool operator<(const Uint& a, const Uint& b)
	{
		return (a.Compare(b) < 0);
	}
	friend bool operator>(const Uint& a, const Uint& b)
	{
		return (a.Compare(b) > 0);
	}
};

template <typename T>
class ArithType {
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
	
	virtual bool operator==(const ArithType& b)
	{
		return (value_ == b.value_);
	}
	virtual bool operator!=(const ArithType& b)
	{
		return !(*this == b);
	}
	virtual bool operator<(const ArithType& b)
	{
		return (value_ < b.value_);
	}
	virtual bool operator<=(const ArithType& b)
	{
		return (value_ <= b.value_);
	}
	virtual bool operator>(const ArithType& b)
	{
		return (value_ > b.value_);
	}
	virtual bool operator>=(const ArithType& b)
	{
		return (value_ >= b.value_);
	}
	
	virtual ArithType operator+(const T& b)
	{
		assert((b > 0 && value_ + b <= std::numeric_limits<T>::max()) ||
			   (b < 0 && value_ + b >= std::numeric_limits<T>::min()));
		return ArithType(value_+b);
	}
	virtual ArithType& operator+=(const T& b)
	{
		assert((b > 0 && value_ + b <= std::numeric_limits<T>::max()) ||
			   (b < 0 && value_ + b >= std::numeric_limits<T>::min()));
		value_ += b;
		return *this;
	}
	virtual ArithType operator-(const T& b)
	{
		assert((b > 0 && value_ - b >= std::numeric_limits<T>::min()) ||
			   (b < 0 && value_ - b <= std::numeric_limits<T>::max()));
		return ArithType(value_-b);
	}
	virtual ArithType& operator-=(const T& b)
	{
		assert((b > 0 && value_ - b >= std::numeric_limits<T>::min()) ||
			   (b < 0 && value_ - b <= std::numeric_limits<T>::max()));
		value_ -= b;
		return *this;
	}
	
	virtual ArithType operator+(const ArithType& b)
	{
		return operator+(b.value_);
	}
	virtual ArithType operator-(const ArithType& b)
	{
		return operator-(b.value_);
	}
	virtual ArithType& operator+=(const ArithType& b)
	{
		return operator+=(b.value_);
	}
	virtual ArithType& operator-=(const ArithType& b)
	{
		return operator-=(b.value_);
	}
	
	virtual std::enable_if_t<std::is_integral<T>::value, ArithType> operator&(const T& b)
	{
		return ArithType(value_ & b);
	}
	virtual std::enable_if_t<std::is_integral<T>::value, ArithType&> operator&=(const T& b)
	{
		value_ &= b;
		return *this;
	}
	virtual std::enable_if_t<std::is_integral<T>::value, ArithType> operator|(const T& b)
	{
		return ArithType(value_ & b);
	}
	virtual std::enable_if_t<std::is_integral<T>::value, ArithType&> operator|=(const T& b)
	{
		value_ |= b;
		return *this;
	}
	
	virtual std::enable_if_t<std::is_integral<T>::value, ArithType> operator&(const ArithType& b)
	{
		return ArithType(value_ & b.value_);
	}
	virtual std::enable_if_t<std::is_integral<T>::value, ArithType&> operator&=(const ArithType& b)
	{
		value_ &= b.value_;
		return *this;
	}
	virtual std::enable_if_t<std::is_integral<T>::value, ArithType> operator|(const ArithType& b)
	{
		return ArithType(value_ | b.value_);
	}
	virtual std::enable_if_t<std::is_integral<T>::value, ArithType&> operator|=(const ArithType& b)
	{
		value_ |= b.value_;
		return *this;
	}
	
	virtual std::enable_if_t<std::is_signed<T>::value, ArithType> operator-()
	{
		return ArithType(-value_);
	}
	
protected:
	ArithType()
	{
		ASSERT_ARITHMETIC(T);
	}
	
private:
	ArithType(const T& v)
		: value_(v) {}
	
	ArithType& operator=(const ArithType& v)
	{
		value_ = v.value_;
		return *this;
	}
	
	T value_;
};

#endif // BTCLITE_ARITHMETIC_H