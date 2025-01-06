#pragma once

#include "daxa/daxa.inl"

struct Settings
{
    daxa_f32vec4 backgroundColor;
};

struct ComputePush
{
    Settings settings;
    daxa_ImageViewId image;
    daxa_u32vec2 frame_dim;
    daxa_f32 time;
};