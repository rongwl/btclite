#include "compact.h"

namespace btclite {
namespace consensus {

inline size_t LogicalSize(util::uint256_t value)
{
    size_t byte = 0;

    for (; value != 0; ++byte)
        value >>= 8;

    return byte;
}

void Compact::SetCompact(uint32_t compact)
{
    size_t size = compact >> 24;
    uint32_t word = compact & 0x007fffff;
    
    if (size <= 3) {
        word >>= 8 * (3 - size);
        normal_ = word;
    } else {
        normal_ = word;
        normal_ <<= 8 * (size - 3);
    }

    negative_ = (word != 0 && (compact & 0x00800000) != 0);
    overflowed_ = (word != 0 && ((size > 34) ||
                                (word > 0xff && size > 33) ||
                                (word > 0xffff && size > 32)));
}

void Compact::GetCompact()
{
    size_t size = LogicalSize(normal_);
    
    if (size <= 3) {
        compact_ = static_cast<uint64_t>(normal_) << 8 * (3 - size);
    } else {
        compact_ = static_cast<uint64_t>(normal_ >> 8 * (size - 3));
    }
    
    // The 0x00800000 bit denotes the sign.
    // Thus, if it is already set, divide the mantissa by 256 and increase the exponent.
    if (compact_ & 0x00800000) {
        compact_ >>= 8;
        size++;
    }
    
    assert((compact_ & ~0x007fffff) == 0);
    assert(size < 256);
    compact_ |= size << 24;
}

} // namespace consensus
} // namespace btclite
