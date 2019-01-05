#ifndef BTCLITE_UINT256_H
#define BTCLITE_UINT256_H

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

/** Template base class for fixed-sized opaque blobs. */
template <unsigned int BITS>
class BaseBlob {
public:
	BaseBlob()
	{
		SetNull();
	}
	
	bool IsNull() const
    {
        for (int i = 0; i < WIDTH; i++)
            if (data[i] != 0)
                return false;
        return true;
    }
	
	void SetNull()
	{
		std::memset(data, 0, sizeof data);
	}
	
	inline int Compare(const BaseBlob& b) const
	{
		return memcmp(data, b.data, sizeof(data));
	}
	
	friend inline bool operator==(const BaseBlob& a, const BaseBlob& b)
	{
		return a.Compare(b) == 0;
	}
	friend inline bool operator!=(const BaseBlob& a, const BaseBlob& b)
	{
		return !(a == b);
	}
	
	std::string GetHex() const;
    void SetHex(const std::string& str);
    std::string ToString() const;

    unsigned char* begin()
    {
        return &data[0];
    }

    unsigned char* end()
    {
        return &data[WIDTH];
    }

    const unsigned char* begin() const
    {
        return &data[0];
    }

    const unsigned char* end() const
    {
        return &data[WIDTH];
    }

    unsigned int size() const
    {
        return sizeof(data);
    }

    uint64_t GetUint64(int pos) const
    {
        const uint8_t* ptr = data + pos * 8;
        return ((uint64_t)ptr[0]) | \
               ((uint64_t)ptr[1]) << 8 | \
               ((uint64_t)ptr[2]) << 16 | \
               ((uint64_t)ptr[3]) << 24 | \
               ((uint64_t)ptr[4]) << 32 | \
               ((uint64_t)ptr[5]) << 40 | \
               ((uint64_t)ptr[6]) << 48 | \
               ((uint64_t)ptr[7]) << 56;
    }
	
protected:
	static constexpr int WIDTH = BITS / 8;
	std::byte data[WIDTH];
};

class uint256 : public BaseBlob<256> {
public:
	uint256() {}
};

#endif // BTCLITE_UINT256_H
