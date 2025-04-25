#pragma once

#include "engine.inl"
#include "const.inl"
#include "bricks.inl"
#include "chunks.inl"
#include "material.inl"
#include "shared.inl"
#include "ray.inl"

struct ImageBuffer
{
    daxa_ImageViewId voxelUVs;
    daxa_ImageViewId brickUVs;
    daxa_ImageViewId ao;
    daxa_ImageViewId aoDenoised;
    daxa_ImageViewId indirectLight;
    daxa_ImageViewId directLight;
    daxa_ImageViewId history;
    daxa_ImageViewId currentFrame;
};

DAXA_DECL_BUFFER_PTR(ImageBuffer);

struct WorldData
{
    daxa_f32vec3 sunDir;
};

DAXA_DECL_BUFFER_PTR(WorldData);

struct RenderPush : EnginePush
{
    daxa_BufferPtr(ImageBuffer) gbuffer;
    daxa_BufferPtr(Chunks) chunksBuffer;
    daxa_BufferPtr(Bricks) bricksBuffer;
    daxa_BufferPtr(BrickPointers) brickPtrBuffer;
    daxa_BufferPtr(Materials) materialsBuffer;
    daxa_BufferPtr(MaterialPointers) materialPtrBuffer;
    daxa_BufferPtr(WorldData) stateBuffer;
};
