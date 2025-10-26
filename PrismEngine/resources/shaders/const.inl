#pragma once
#include "daxa/daxa.inl"

// Material constants
static const daxa_u32 BITS_PER_MATERIAL = 4;
static const daxa_u32 TOTAL_MATERIALS = 1 << BITS_PER_MATERIAL;


static const daxa_u32 BITS_PER_BYTE = 8;
static const float EPSILON = 0.0001;
static const float PI = 3.14159265359;
static const daxa_u32 MAX_UINT32 = 0xFFFFFFFF;
static const daxa_u64 MAX_UINT64 = 0xFFFFFFFFFFFFFFFF;
static const daxa_f32 MAX_FLOAT = 3.402823466e+38f;
static const daxa_f32 MIN_FLOAT = -3.402823466e+38f;

static const daxa_u32 LIGHTING_DOWNSAMPLE = 2;
static const daxa_f32 LIGHTING_UPSAMPLE = 1.0 / LIGHTING_DOWNSAMPLE;
