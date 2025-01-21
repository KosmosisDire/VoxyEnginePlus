#pragma once

#include "daxa/daxa.inl"
#include "voxel-hashmap.inl"
#include "const.inl"


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
    daxa_u32 showHitPos = 0;
    daxa_u32 showUVs = 0;
    daxa_u32 showDepth = 0;
    daxa_u32 showDepthPrepass = 0;
    daxa_u32 beamOptimization = 1;
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
    BrickBitmask occupancy[TOTAL_BRICKS];
};
DAXA_DECL_BUFFER_PTR(BrickOccupancy);

struct ChunkOccupancy
{
    daxa_u32vec2 occupancy[TOTAL_CHUNKS];
};
DAXA_DECL_BUFFER_PTR(ChunkOccupancy);

struct GBuffer
{
    daxa_ImageViewId color;
    daxa_ImageViewId normal;
    daxa_ImageViewId position;
    daxa_ImageViewId indirect; // indirect light (global illumination)
    daxa_ImageViewId depth;
    daxa_ImageViewId depthHalfRes;
    daxa_ImageViewId voxelIDs; // global brick id and local voxel id
};

struct ComputePush
{
    GBuffer gbuffer;
    daxa_ImageViewId final_image;
    daxa_ImageViewId blueNoise; // blue noise texture 128 x 128, different for each frame
    daxa_BufferPtr(ChunkOccupancy) chunk_occupancy_ptr;
    daxa_BufferPtr(BrickOccupancy) brick_occupancy_ptr;
    daxa_BufferPtr(VoxelHashmap) voxel_hashmap_ptr;
    daxa_BufferPtr(VoxelHashmap) past_voxel_hashmap_ptr;
    daxa_BufferPtr(RenderData) state_ptr;
    daxa_u32vec2 frame_dim;
};

struct DenoisePush
{
    GBuffer gbuffer;
    daxa_u32 pass;
    daxa_u32vec2 frame_dim;
}; 
