#ifndef BTCLITE_UINT256_H
#define BTCLITE_UINT256_H

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
		std::memset(this, 0, WIDTH);
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
	
	std::string GetHex() const;
    void SetHex(const std::string& str);
protected:
	static constexpr int WIDTH = BITS / 8;
};

class uint256 : public BaseBlob<256> {
public:
	uint256() {}
};

#endif // BTCLITE_UINT256_H
