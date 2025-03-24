#pragma once

#include "daxa/daxa.inl"
#include "const.inl"

// Core ray structure
struct Ray
{
    daxa_f32vec3 origin;
    daxa_f32vec3 direction;
    daxa_f32 maxDist;

#ifndef __cplusplus
    // Add these constants for jittering
    static const int HALTON_SEQUENCE_LENGTH = 16; // Number of jitter patterns to cycle through
    static const float JITTER_SCALE = 0.5f; // Size of jitter (0.5 = half a pixel)

    // Halton sequence for jittering (base 2 and 3)
    // These provide a well-distributed sampling pattern
    static const float2 HALTON_SAMPLES[HALTON_SEQUENCE_LENGTH] =
    {
        float2(0.0f, 0.0f),
        float2(0.5f, 0.33333f),
        float2(0.25f, 0.66666f),
        float2(0.75f, 0.11111f),
        float2(0.125f, 0.44444f),
        float2(0.625f, 0.77777f),
        float2(0.375f, 0.22222f),
        float2(0.875f, 0.55555f),
        float2(0.0625f, 0.88888f),
        float2(0.5625f, 0.03703f),
        float2(0.3125f, 0.37037f),
        float2(0.8125f, 0.70370f),
        float2(0.1875f, 0.14814f),
        float2(0.6875f, 0.48148f),
        float2(0.4375f, 0.81481f),
        float2(0.9375f, 0.25925f)
    };

    // Get jitter offset for current frame
    static float2 getJitterOffset(uint frameIndex)
    {
        // Cycle through the halton sequence based on frame number
        uint sampleIndex = frameIndex % HALTON_SEQUENCE_LENGTH;
        // Convert from [0,1] to [-0.5,0.5] range and scale by jitter amount
        return (HALTON_SAMPLES[sampleIndex] - 0.5f) * JITTER_SCALE;
    }

    static Ray FromPixelJitter(uint2 pixel, uint2 dimensions, CameraData camera)
    {
        Ray ray;
        ray.origin = camera.position;
        ray.maxDist = camera.far;

        // Get jitter offset for current frame
        float2 jitter = getJitterOffset(p.state_ptr.frame);

        // Convert pixel to NDC space (-1 to 1) with jitter
        float2 uv = (float2(pixel) + 0.5f + jitter) / float2(dimensions);
        float2 ndc = uv * 2.0f - 1.0f;

        // Transform NDC position to world space
        float4 worldPos = mul(camera.invViewProj, float4(ndc.x, ndc.y, 1.0f, 1.0f));
        worldPos.xyz /= worldPos.w;

        // Calculate ray direction
        ray.direction = normalize(worldPos.xyz - camera.position);
        return ray;
    }

    static Ray FromPixel(uint2 pixel, uint2 dimensions, CameraData camera)
    {
        Ray ray;
        ray.origin = camera.position;
        ray.maxDist = camera.far;

        // Convert pixel to NDC space (-1 to 1)
        float2 uv = (float2(pixel) + 0.5f) / float2(dimensions);
        float2 ndc = uv * 2.0f - 1.0f;

        // Transform NDC position to world space
        float4 worldPos = mul(camera.invViewProj, float4(ndc.x, ndc.y, 1.0f, 1.0f));
        worldPos.xyz /= worldPos.w;

        // Calculate ray direction
        ray.direction = normalize(worldPos.xyz - camera.position);
        return ray;
    }

#endif
};

// Result from a single ray trace
struct TraceResult
{
    bool hit;
    daxa_f32vec3 position; // World position of hit
    daxa_f32vec3 normal;   // Surface normal
    daxa_f32 distance;  // Distance from ray origin
    daxa_u32 materialId; // Material of hit surface
    daxa_i32vec3 cell;       // Absolute grid position
    daxa_i32 chunkIndex;  // Index in chunk array
    daxa_i32 brickIndex;  // Index in brick array
    daxa_i32 voxelIndex;  // Index in voxel array
    daxa_f32vec3 voxelUVW; // UVW coordinates in voxel
    daxa_f32vec3 brickUVW; // UVW coordinates in brick
    int iterations;
};

// Final output per pixel
struct PixelResult
{
    daxa_f32vec3 color;      // Final combined color with GI
    daxa_f32vec3 normal;     // Final surface normal
    daxa_f32 depth;       // Closest hit depth
    daxa_f32vec3 position;   // World space position
    daxa_u32 hashmapIndex; // Index into lighting hashmap
    daxa_i32vec2 voxelId;
};

// Constants
static const daxa_f32 MAX_TRACE_DISTANCE = 100.0;
static const daxa_i32 MAX_TRACE_STEPS = 512;

const int RAYCASTER_MAX_RAYS = 256;
struct RayRequests
{
    Ray rays[RAYCASTER_MAX_RAYS];
    daxa_u32 numRays;
};

struct RayResults
{
    TraceResult hits[RAYCASTER_MAX_RAYS];
    daxa_u32 numHits;
};

DAXA_DECL_BUFFER_PTR(RayRequests);
DAXA_DECL_BUFFER_PTR(RayResults);


struct CameraData
{
    daxa_f32mat4x4 viewProj;
    daxa_f32mat4x4 view;
    daxa_f32mat4x4 proj;
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
    daxa_u32 data[8];
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
    daxa_ImageViewId motion;
    daxa_ImageViewId depth;
    daxa_ImageViewId voxelIDs; // global brick id and local voxel id
    daxa_ImageViewId voxelFaceIDs; // material id
    daxa_ImageViewId materialIDs; // material id

    daxa_ImageViewId history;
    daxa_ImageViewId currentFrame;

};

struct ComputePush
{
    daxa_BufferPtr(GBuffer) gbuffer;
    daxa_BufferPtr(ChunkOccupancy) chunk_occupancy_ptr;
    daxa_BufferPtr(BrickOccupancy) brick_occupancy_ptr;
    daxa_BufferPtr(Materials) materials_ptr;
    daxa_BufferPtr(VoxelMaterials) voxel_materials_ptr;
    daxa_BufferPtr(RenderData) state_ptr;
    daxa_BufferPtr(RayRequests) ray_requests_ptr;
    daxa_BufferPtr(RayResults) ray_results_ptr;
    daxa_ImageViewId final_image;
    daxa_ImageViewId blueNoise; // blue noise texture 128 x 128, different for each frame
    daxa_u32vec2 frame_dim;
    daxa_u32 pass;
};

struct DenoisePush
{
    daxa_BufferPtr(GBuffer) gbuffer;
    daxa_u32 pass;
    daxa_u32vec2 frame_dim;
};
