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

struct Gbuffer
{
    daxa_ImageViewId albedo;
    daxa_ImageViewId normal;
    daxa_ImageViewId position;
    daxa_ImageViewId depth;
    daxa_ImageViewId motion;
    daxa_ImageViewId material;
};

DAXA_DECL_BUFFER_PTR(Gbuffer);

struct EngineData
{
    daxa_f32 dt;
    daxa_f32 time;
    daxa_u32 frame;
    daxa_u32 passNum;
    CameraData currentCamera;
    CameraData currentCameraPrev;
    daxa_ImageViewId blueNoise; // blue noise texture 128 x 128, different for each frame
    daxa_ImageViewId blueNoiseStatic; // blue noise texture 128 x 128
};

DAXA_DECL_BUFFER_PTR(EngineData);

struct EnginePush
{
    daxa_BufferPtr(EngineData) engineData;
    daxa_BufferPtr(Gbuffer) gbuffer;
    daxa_ImageViewId screen;
    daxa_u32vec2 screenSize;
};

