#pragma once
#include "daxa/daxa.inl"


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
