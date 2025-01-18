#pragma once

#include "daxa/daxa.inl"

static const daxa_u32 GRID_SIZE = 100;
static const daxa_u32 GRID_SIZE_SQUARE = GRID_SIZE * GRID_SIZE;
static const daxa_u32 GRID_SIZE_CUBE = GRID_SIZE * GRID_SIZE * GRID_SIZE;

static const daxa_u32 BRICK_SIZE = 6;
static const daxa_u32 BRICK_SIZE_SQUARE = BRICK_SIZE * BRICK_SIZE;
static const daxa_u32 BRICK_SIZE_CUBE = BRICK_SIZE * BRICK_SIZE * BRICK_SIZE;

static const daxa_u32 CHUNK_SIZE = 4;
static const daxa_u32 CHUNK_SIZE_SQUARE = CHUNK_SIZE * CHUNK_SIZE;
static const daxa_u32 CHUNK_SIZE_CUBE = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

static const daxa_f32 WORLD_VOXEL_SIZE = 1.0 / (BRICK_SIZE * CHUNK_SIZE);
static const daxa_f32 BRICK_VOXEL_SIZE = 1.0 / BRICK_SIZE;

static const daxa_u32 BITS_PER_BYTE = 8;

struct CameraData
{
    daxa_f32mat4x4 viewProj;
    daxa_f32mat4x4 invViewProj;
    daxa_f32vec3 position;
    float near;
    float far;
};

struct GraphicsSettings
{
    daxa_u32 skyLight = 1;
    daxa_u32 showNormals = 0;
    daxa_u32 showSteps = 0;
    daxa_u32 showUvs = 0;
};

struct RenderData
{
    daxa_f32vec3 sunDir;
    daxa_f32 dt;
    CameraData camera;
    daxa_f32 time;
    daxa_u32 frame;
    GraphicsSettings settings;
};

// 216 bits used for bitmask. Aligned to 256 bits.
// leaving 40 bits.
// last 4 bytes are used for the compressed data ptr (so only 8 bits are wasted per block)
// in the future this could be used for light level accumulation.
struct BrickBitmask
{
    daxa_u32vec4 part1;
    daxa_u32vec3 part2;
    daxa_u32 compressed_data_ptr;
};
struct BrickOccupancy
{
    BrickBitmask occupancy[];
};
DAXA_DECL_BUFFER_PTR(BrickOccupancy);

struct ChunkOccupancy
{
    daxa_u64 occupancy[];
};
DAXA_DECL_BUFFER_PTR(ChunkOccupancy);

struct ComputePush
{
    daxa_ImageViewId image;
    daxa_BufferPtr(ChunkOccupancy) chunk_occupancy_ptr;
    daxa_BufferPtr(BrickOccupancy) brick_occupancy_ptr;
    daxa_BufferPtr(RenderData) state_ptr;
    daxa_u32vec2 frame_dim;
};