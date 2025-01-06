#pragma once

#include "daxa/daxa.inl"

struct Settings
{
    daxa_f32vec4 backgroundColor;
};

struct Occupancy
{
    daxa_u64 occupancy[];
};
DAXA_DECL_BUFFER_PTR(Occupancy);

struct ComputePush
{
    Settings settings;
    daxa_ImageViewId image;
    daxa_BufferPtr(Occupancy) occupancy;
    daxa_u32vec2 frame_dim;
    daxa_f32 time;
};