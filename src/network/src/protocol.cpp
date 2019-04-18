#include "protocol.h"


size_t VarIntSize(size_t vec_size)
{
    if (vec_size < varint_16bits)
        return 1;
    else if (vec_size <= std::numeric_limits<uint16_t>::max())
        return 3;
    else if (vec_size <= std::numeric_limits<uint32_t>::max())
        return 5;
    else
        return 9;
}


