#include "atmosphere.slang"
#include "lygia/color/tonemap/unreal.hlsl"
#include "common.slang"


[numthreads(8, 8, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    uint2 pixel = pixel_i.xy;
    if (pixel.x >= p.screenSize.x || pixel.y >= p.screenSize.y)
        return;

    // Retrieve our render targets and G-buffer textures.
    RWTexture2D<float4> screenTex = RWTexture2D<float4>.get(p.screen);
    RWTexture2D<float3> albedoTex = RWTexture2D<float3>.get(p.gbuffer.color);
    RWTexture2D<uint2> voxelIDTex = RWTexture2D<uint2>.get(p.gbuffer.voxelIDs);
    RWTexture2D<uint> voxelFaceIDTex = RWTexture2D<uint>.get(p.gbuffer.voxelFaceIDs);
    RWTexture2D<uint> materialIDTex = RWTexture2D<uint>.get(p.gbuffer.materialIDs);
    RWTexture2D<float2> voxelUVsTex = RWTexture2D<float2>.get(p.gbuffer.voxelUVs);
    RWTexture2D<float2> brickUVsTex = RWTexture2D<float2>.get(p.gbuffer.brickUVs);
    RWTexture2D<float3> normalTex = RWTexture2D<float3>.get(p.gbuffer.normal);
    RWTexture2D<float3> positionTex = RWTexture2D<float3>.get(p.gbuffer.position);
    RWTexture2D<float2> motionTex = RWTexture2D<float2>.get(p.gbuffer.motion);
    RWTexture2D<float> depthTex = RWTexture2D<float>.get(p.gbuffer.depth);
    RWTexture2D<float3> historyTex = RWTexture2D<float3>.get(p.gbuffer.history);

    uint2 voxelID = voxelIDTex[pixel];
    uint voxelFaceID = voxelFaceIDTex[pixel];

    // Get surface properties.
    float3 normal = normalTex[pixel];
    float2 brickUV = brickUVsTex[pixel];
    float3 albedo = albedoTex[pixel];
    float3 position = positionTex[pixel];
    float2 motion = motionTex[pixel];
    float depth = depthTex[pixel];
    float3 history = historyTex[pixel];


    // If there is no valid normal, we assume sky.
    if (depth < 0)
    {
        float3 sky = getAtmosphereColor(initAtmosphere(Ray.FromPixel(pixel, p.screenSize, p.stateBuffer.camera).direction, p.stateBuffer.sunDir), p.stateBuffer.time);
        screenTex[pixel] = tonemapUnreal(float4(sky, 1.0));
        return;
    }

    float3 color = albedo;

    // Apply tonemapping.
    color = tonemapUnreal(color);

    screenTex[pixel] = float4(color, 1.0);
}
