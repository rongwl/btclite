#ifndef BTCLITE_Hash256_H
#define BTCLITE_Hash256_H

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

#include "Endian.h"

/** Template base class for fixed-sized opaque blobs. */
template <unsigned int BITS>
class BaseBlob : public Bytes<BITS/8>{
public:
	BaseBlob()
	{
		SetNull();
	}
	
	bool IsNull() const
    {
		for (auto it : *this) {
			if (it != 0)
				return false;
		}
		return true;
    }
	
	void SetNull()
	{
		std::memset(&this->front(), 0, WIDTH);
	}
	
	int Compare(const BaseBlob& b) const
	{
		return std::memcmp(&this->front(), &b.front(), WIDTH);
	}
		
    std::string ToString() const
	{
		return GetHex();
	}

    uint64_t GetUint64(int pos) const
    {
        auto it = this->begin() + pos * 8;
        return *(it) | 
			   (*(it+1) << 8) | 
               (*(it+2) << 16) | 
               (*(it+3) << 24) | 
               (*(it+4) << 32) | 
               (*(it+5) << 40) | 
               (*(it+6) << 48) | 
               (*(it+7) << 56);
    }
	
	template <typename SType>
	void Serialize(SType& s) const
	{
		s.write(reinterpret_cast<const char*>(&this->front()), this->size());
	}
	template <typename SType>
	void UnSerialize(SType& s)
	{
		s.read(reinterpret_cast<char*>(&this->front()), this->size());
	}
	
	std::string GetHex() const;
    void SetHex(const std::string& str);
protected:
	static constexpr int WIDTH = BITS / 8;
};

template <unsigned int BITS>
class Uint : public BaseBlob<BITS> {
public:
	Uint() {}
	
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
using Hash256 = Uint<256>;

#endif // BTCLITE_Hash256_H
