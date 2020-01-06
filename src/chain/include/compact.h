#ifndef BTCLITE_COMPACT_H
#define BTCLITE_COMPACT_H


#include "arithmetic.h"


namespace btclite {
namespace chain {

/*
 * The "compact" format is a representation of a whole
 * number N using an unsigned 32bit number similar to a
 * floating point format.
 * The most significant 8 bits are the unsigned exponent of base 256.
 * This exponent can be thought of as "number of bytes of N".
 * The lower 23 bits are the mantissa.
 * Bit number 24 (0x800000) represents the sign of N.
 * N = (-1^sign) * mantissa * 256^(exponent-3)
 *
 * Satoshi's original implementation used BN_bn2mpi() and BN_mpi2bn().
 * MPI uses the most significant bit of the first byte as sign.
 * Thus 0x1234560000 is compact (0x05123456)
 * and  0xc0de000000 is compact (0x0600c0de)
 *
 * Bitcoin only uses this "compact" format for encoding difficulty
 * targets, which are unsigned 256bit quantities.  Thus, all the
 * complexities of the sign bit and using base 256 are probably an
 * implementation accident.
*/
class Compact {
public:
    // Construct from a 32 bit compact number.
    explicit Compact(uint32_t compact)
    {
        SetCompact(compact);
        GetCompact();
    }

    // Construct from a 256 bit number
    explicit Compact(const util::uint256_t& normal)
        : normal_(normal)
    {
        GetCompact();
    }
    
    //-------------------------------------------------------------------------
    static bool Negative(uint32_t compact)
    {
        return ((compact & 0x007fffff) && (compact & 0x00800000));
    }
    
    //-------------------------------------------------------------------------
    const util::uint256_t& normal() const
    {
        return normal_;
    }
    
    uint32_t compact() const
    {
        return compact_;
    }
        
    bool overflowed() const
    {
        return overflowed_;
    }
    
private:
    util::uint256_t normal_ = 0;
    uint32_t compact_ = 0;
    bool overflowed_ = false;
    
    void SetCompact(uint32_t compact);
    void GetCompact();
};

} // namespace chain
} // namespace btclite

#endif // BTCLITE_COMPACT_H
