#include "daxa/daxa.inl"

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