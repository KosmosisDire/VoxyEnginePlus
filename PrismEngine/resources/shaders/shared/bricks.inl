#pragma once
#include "daxa/daxa.inl"
#include "const.inl"

// 216 bits used for bitmask. Aligned to 256 bits.
// leaving 40 bits.
// last 4 bytes are used for the compressed data ptr (so only 8 bits are wasted per block)
struct BrickBitmask
{
    daxa_u32 data[8];
};

struct Bricks
{
    BrickBitmask data[256];
};

DAXA_DECL_BUFFER_PTR(Bricks);

struct BrickPointers
{
#ifdef __cplusplus
    daxa_u32 data[TOTAL_BRICKS / 4];
#else
    uint8_t data[TOTAL_BRICKS];
#endif
};

DAXA_DECL_BUFFER_PTR(BrickPointers);