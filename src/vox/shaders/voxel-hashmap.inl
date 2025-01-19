#pragma once

#include "daxa/daxa.inl"
#include "const.inl"

static const daxa_u32 HASH_TABLE_SIZE = 2457 * GRID_SIZE_SQUARE;

struct VoxelHashmapEntry
{
    daxa_u32 visibleCount;
    daxa_u32 globalBrickIndex;
    daxa_u32 localVoxelIndex;
    daxa_f32vec3 indirectLight;
};

struct VoxelHashmap
{
    VoxelHashmapEntry entries[HASH_TABLE_SIZE];
};

struct VoxelQueue
{
    daxa_u32 count;
    daxa_u32 padding[3];
    daxa_u32 hashmapIndices[HASH_TABLE_SIZE];
};

DAXA_DECL_BUFFER_PTR(VoxelHashmap);
DAXA_DECL_BUFFER_PTR(VoxelQueue);
