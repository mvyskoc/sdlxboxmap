#include <cstdint>

#include "bitext.h"
#define __BITEXT_CPP

bool is_little_endian()
{
    union {
        uint32_t i;
        char c[4];
    } bint = { .i = 0x12345678};

    return bint.c[0] == 0x78; 
}

int to_int16le(int value)
{
    value &= 0xffff;
    if (!is_little_endian()) {
        char lo_byte = value & 0xff;
        value >>= 8;
        value |= (lo_byte << 8);
    }
    return value;
}
