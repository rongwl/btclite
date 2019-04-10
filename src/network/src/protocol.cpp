#include "protocol.h"


size_t VarIntSize(size_t vec_size)
{
    if (vec_size < varint_16bits)
        return 1;
    else if (vec_size <= UINT16_MAX)
        return 3;
    else if (vec_size <= UINT32_MAX)
        return 5;
    else
        return 9;
}

bool MessageHeader::IsValid(uint32_t magic) const
{
	return true;
}

