#pragma once

#include "daxa/daxa.inl"
#include "const.inl"

static const daxa_u32 HASH_TABLE_SIZE = 16777216;
static const daxa_u32 EMPTY_KEY = 0;

struct VoxelHashmap
{
    daxa_u32 hitCounts[HASH_TABLE_SIZE];
    daxa_u32 brickIndices[HASH_TABLE_SIZE];
    daxa_u32 voxelIndices[HASH_TABLE_SIZE];
    daxa_f32vec3 indirectLight[HASH_TABLE_SIZE];
};

DAXA_DECL_BUFFER_PTR(VoxelHashmap);

