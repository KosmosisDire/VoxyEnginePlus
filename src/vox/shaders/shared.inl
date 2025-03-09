#pragma once
#include "daxa/daxa.inl"

#define GRID_SIZE 32
#define CHUNK_SIZE 32
#define WORLD_SIDE_LENGTH (GRID_SIZE * CHUNK_SIZE)
#define NUM_VOXELS (WORLD_SIDE_LENGTH * WORLD_SIDE_LENGTH)

struct Lighting
{
    daxa_f32vec3 light[NUM_VOXELS];
};

struct Render
{
    daxa_f32vec3 color[NUM_VOXELS];
};

DAXA_DECL_BUFFER_PTR(Lighting);
DAXA_DECL_BUFFER_PTR(Render);

struct ComputePush
{
    daxa_BufferPtr(Lighting) lightingBuffer;
    daxa_BufferPtr(Render) renderBuffer;
    daxa_ImageViewId screen;
    daxa_u32vec2 frame_dim;
    daxa_f32 time;
};