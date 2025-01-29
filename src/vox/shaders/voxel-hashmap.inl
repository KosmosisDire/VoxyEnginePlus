#pragma once

#include "daxa/daxa.inl"
#include "const.inl"

static const daxa_u32 HASH_TABLE_SIZE = 16777216 / 8;
static const daxa_u32 EMPTY_KEY = 0;

struct HashmapEntry
{
    daxa_u32 temporalFrame;
    daxa_u32 hitCount;
    daxa_u32 brickIndex;
    daxa_u32 voxelIndex;
    daxa_f32vec3 indirectLight;
};

struct VoxelHashmap
{
    HashmapEntry entries[HASH_TABLE_SIZE];
};

DAXA_DECL_BUFFER_PTR(VoxelHashmap);

