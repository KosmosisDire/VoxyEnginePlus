#pragma once

#include <daxa/daxa.inl>

struct NodesPtr
{
    daxa_u32vec3 nodes[];
};

DAXA_DECL_BUFFER_PTR(NodesPtr);

struct DataPtr
{
    uint8_t data[];
};

DAXA_DECL_BUFFER_PTR(DataPtr);


struct VoxelMap
{
    uint32_t TreeScale;
    daxa_RWBufferPtr(NodesPtr) nodes;
    daxa_RWBufferPtr(DataPtr) data;
};

// Push constants for compute shaders
struct ComputePush
{
    VoxelMap voxelMap;
    daxa_ImageViewId screen;
    daxa_u32vec2 frame_dim;
    daxa_f32 time;
    uint32_t frame_number;
    daxa_f32vec3 cameraPos;
    daxa_f32mat4x4 invViewProjMat;
};
