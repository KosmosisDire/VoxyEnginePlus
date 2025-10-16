#pragma once

#include "engine.inl"
#include "const.inl"

// Core ray structure
struct Ray
{
    daxa_f32vec3 origin;
    daxa_f32vec3 direction;
    daxa_f32 maxDist;
    daxa_u32 maxIterations = 1024;

#ifndef __cplusplus
    // Add these constants for jittering
    static const int HALTON_SEQUENCE_LENGTH = 16; // Number of jitter patterns to cycle through
    static const float JITTER_SCALE = 0.6f; // Size of jitter (0.5 = half a pixel)

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
        ray.maxDist = camera.far * 50;

        // Get jitter offset for current frame
        float2 jitter = getJitterOffset(p.stateBuffer.frame);

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
        ray.maxDist = camera.far * 50;

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
    GraphicsSettings settings;
};

// 216 bits used for bitmask. Aligned to 256 bits.
// leaving 40 bits.
// last 4 bytes are used for the compressed data ptr (so only 8 bits are wasted per block)
struct BrickBitmask
{
    daxa_u32 data[8];
};

struct Bricks
{
    BrickBitmask data[256];
};

DAXA_DECL_BUFFER_PTR(Bricks);

struct BrickPointers
{
#ifdef __cplusplus
    daxa_u32 data[TOTAL_BRICKS / 4];
#else
    uint8_t data[TOTAL_BRICKS];
#endif
};

DAXA_DECL_BUFFER_PTR(BrickPointers);

struct Chunks
{
    daxa_u32vec2 data[TOTAL_CHUNKS];
};

DAXA_DECL_BUFFER_PTR(Chunks);

//=============================================================================
// Contree Structures (4x4x4 sparse variable depth tree)
//=============================================================================

// Contree constants
static const daxa_u32 CONTREE_LEVELS = 4;
static const daxa_u32 CONTREE_BRANCHING = 4;
static const daxa_u32 CONTREE_CHILDREN = 64; // 4x4x4
static const daxa_u32 CONTREE_INVALID_PTR = 0xFFFFFFFF;

// Contree node - 16 bytes
// Bytes 0-3:   PackedData[0] = IsLeaf(1) | IsAbsolutePtr(1) | ChildPtr(30)
// Bytes 4-7:   PackedData[1] = PopMask low 32 bits
// Bytes 8-11:  PackedData[2] = PopMask high 32 bits
// Bytes 12-15: PackedData[3] = MaterialId (16 bits) + padding (16 bits)
struct ContreeNode
{
    daxa_u32 PackedData[4];  // 16 bytes total

#ifndef __cplusplus
    property bool IsLeaf {
        get { return (PackedData[0] & 1) != 0; }
    }
    property uint ChildPtr {
        get { return PackedData[0] >> 2; }
    }
    property uint64_t PopMask {
        get { return PackedData[1] | uint64_t(PackedData[2]) << 32; }
    }
    property uint MaterialId {
        get { return PackedData[3] & 0xFFFF; }
    }
#endif
};

struct ContreeBuffer
{
    ContreeNode nodes[1]; // Variable length array
};

DAXA_DECL_BUFFER_PTR(ContreeBuffer);

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
    Material data[TOTAL_MATERIALS];
};

DAXA_DECL_BUFFER_PTR(Materials);

struct MaterialPointers
{
    uint8_t data[TOTAL_BRICKS];
};

DAXA_DECL_BUFFER_PTR(MaterialPointers);

struct GBuffer
{
    daxa_ImageViewId color;
    daxa_ImageViewId normal;
    daxa_ImageViewId position;
    daxa_ImageViewId depth;
    daxa_ImageViewId ao; // ambient occlusion
};

struct GPUTimings
{
    daxa_f32 terrainGen;
    daxa_f32 gbufferTrace;
    daxa_f32 gtaoPass;
    daxa_f32 aoApply;
    daxa_f32 composite;
    daxa_f32 totalFrame;
};

struct ComputePush
{
    daxa_BufferPtr(GBuffer) gbuffer;
    daxa_BufferPtr(Materials) materialsBuffer;
    daxa_BufferPtr(RenderData) stateBuffer;
    daxa_BufferPtr(ContreeBuffer) contreeBuffer;
    daxa_ImageViewId screen;
    daxa_ImageViewId blueNoise;
    daxa_u32vec2 screenSize;
    daxa_u32 passNum;
    daxa_u32 contreeNodeCount;
};
