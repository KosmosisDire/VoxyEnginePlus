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
    float fov;
    // padding
    daxa_f32vec2 padding;
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
    daxa_f32 time;
    daxa_u32 frame;
    // padding
    daxa_f32vec2 padding;
    CameraData camera;
    CameraData lastCamera;
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

// Material definition
struct Material
{
    daxa_f32vec3 albedo;      // Base color
    daxa_f32vec3 emission;    // Emissive color
    daxa_f32vec3 volumetricAbsorbtion;    // Volumetric absorption color
    daxa_f32 transparency; // 0-1 value for transparency
    daxa_f32 reflectivity; // 0-1 value for reflection strength
    daxa_f32 roughness;    // Surface roughness for reflection
    daxa_f32 metallic;     // Metallic factor for reflection color
};

struct Materials
{
    Material materials[TOTAL_MATERIALS];
};
DAXA_DECL_BUFFER_PTR(Materials);

struct VoxelMaterials
{
    daxa_u32 materials[TOTAL_VOXELS / (32 / BITS_PER_MATERIAL)];
};
DAXA_DECL_BUFFER_PTR(VoxelMaterials);

struct GBuffer
{
    daxa_ImageViewId color;
    daxa_ImageViewId normal;
    daxa_ImageViewId position;
    daxa_ImageViewId voxelUVs;
    daxa_ImageViewId brickUVs;
    daxa_ImageViewId indirect; // indirect light (global illumination)
    daxa_ImageViewId indirectLast; // indirect light from last frame
    daxa_ImageViewId indirectDenoised; // indirect light denoised  
    daxa_ImageViewId indirectPerVoxelPass1;
    daxa_ImageViewId indirectPerVoxelPass2; 
    daxa_ImageViewId motion;
    daxa_ImageViewId depth;
    daxa_ImageViewId depthHalfRes;
    daxa_ImageViewId voxelIDs; // global brick id and local voxel id
    daxa_ImageViewId voxelFaceIDs; // material id
    daxa_ImageViewId materialIDs; // material id
    daxa_ImageViewId ssao; // screen space ambient occlusion
    daxa_ImageViewId shadow; // shadow map
};

struct ComputePush
{
    daxa_BufferPtr(GBuffer) gbuffer;
    daxa_BufferPtr(ChunkOccupancy) chunk_occupancy_ptr;
    daxa_BufferPtr(BrickOccupancy) brick_occupancy_ptr;
    daxa_BufferPtr(Materials) materials_ptr;
    daxa_BufferPtr(VoxelMaterials) voxel_materials_ptr;
    daxa_BufferPtr(VoxelHashmap) voxel_hashmap_ptr;
    daxa_BufferPtr(RenderData) state_ptr;
    daxa_ImageViewId final_image;
    daxa_ImageViewId blueNoise; // blue noise texture 128 x 128, different for each frame
    daxa_u32vec2 frame_dim;
    daxa_u32 pass;
};

struct DenoisePush
{
    daxa_BufferPtr(GBuffer) gbuffer;
    daxa_BufferPtr(VoxelHashmap) voxel_hashmap_ptr;
    daxa_u32 pass;
    daxa_u32vec2 frame_dim;
}; 
