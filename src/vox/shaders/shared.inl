#pragma once

#include "daxa/daxa.inl"

static const daxa_u32 GRID_SIZE = 25;
static const daxa_u32 TREE_SIZE = 4;
static const daxa_u32 TREE_SIZE_SQUARE = TREE_SIZE * TREE_SIZE;
static const daxa_u32 TREE_SIZE_CUBE = TREE_SIZE * TREE_SIZE * TREE_SIZE;
static const daxa_u32 BITS_PER_BYTE = 8;

struct CameraData
{
    daxa_f32mat4x4 viewProj;
    daxa_f32mat4x4 invViewProj;
    daxa_f32vec3 position;
    float near;
    float far;
};

struct StateData
{
    daxa_f32vec3 sunDir;
    CameraData camera;
    float dt;
    daxa_f32 time;
};

struct Occupancy
{
    daxa_u64 occupancy[];
};
DAXA_DECL_BUFFER_PTR(Occupancy);

struct ComputePush
{
    daxa_ImageViewId image;
    daxa_BufferPtr(Occupancy) chunk_occupancy_ptr;
    daxa_BufferPtr(Occupancy) brick_occupancy_ptr;
    daxa_BufferPtr(StateData) state_ptr;
    daxa_u32vec2 frame_dim;
};