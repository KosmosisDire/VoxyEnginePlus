#pragma once

#include "daxa/daxa.inl"
#include "const.inl"

static const daxa_u32 HASH_TABLE_SIZE = 2097152;
static const daxa_u32 EMPTY_KEY = 0;

struct VoxelHashmapEntry
{
    daxa_f32vec3 lighting;
    daxa_u32 historyCount;
    daxa_u32 lastContributionFrame;
    daxa_u32 uniqueFaceID;
};

struct VoxelHashmap
{
    daxa_u32 brickIndices[HASH_TABLE_SIZE];
    VoxelHashmapEntry entries[HASH_TABLE_SIZE];
};

DAXA_DECL_BUFFER_PTR(VoxelHashmap);

