#ifndef COMMON_H
#define COMMON_H
#include "../common.slang"

// Core ray structure
struct Ray
{
    float3 origin;
    float3 direction;
    float maxDist;
};

// Result from a single ray trace
struct TraceResult
{
    bool hit;
    float3 position; // World position of hit
    float3 normal;   // Surface normal
    float distance;  // Distance from ray origin
    uint materialId; // Material of hit surface
    int3 cell;       // Absolute grid position
    int chunkIndex;  // Index in chunk array
    int brickIndex;  // Index in brick array
    int voxelIndex;  // Index in voxel array
};

// Combined results from all traced rays
struct SceneHitInfo
{
    TraceResult solid;
    TraceResult transparent;
    TraceResult reflection;
};

// Final output per pixel
struct PixelResult
{
    float3 color;      // Final combined color with GI
    float3 normal;     // Final surface normal
    float depth;       // Closest hit depth
    float3 position;   // World space position
    bool hitAnything;  // Whether any ray hit
    uint hashmapIndex; // Index into lighting hashmap
    int2 voxelId;
};

// Constants
static const float MAX_TRACE_DISTANCE = 100.0;
static const int MAX_TRACE_STEPS = 512;
#endif 