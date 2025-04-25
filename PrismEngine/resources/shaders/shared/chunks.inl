#pragma once
#include "daxa/daxa.inl"
#include "const.inl"

struct Chunks
{
    daxa_u32vec2 data[TOTAL_CHUNKS];
};

DAXA_DECL_BUFFER_PTR(Chunks);
