#pragma once
#include "daxa/daxa.inl"
#include "const.inl"

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