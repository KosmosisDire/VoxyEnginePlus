#include "lygia/color/tonemap/aces.hlsl"
#include "common.slang"

[numthreads(8, 8, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    uint2 pixel = pixel_i.xy;
    if (pixel.x >= p.screenSize.x || pixel.y >= p.screenSize.y)
        return;

    // Get render targets
    RWTexture2D<float4> screenTex = RWTexture2D<float4>.get(p.screen);
    RWTexture2D<float3> colorTex = RWTexture2D<float3>.get(p.gbuffer.color);
    RWTexture2D<float> depthTex = RWTexture2D<float>.get(p.gbuffer.depth);

    float depth = depthTex[pixel];
    float3 color = colorTex[pixel];

    // Simple sky for background
    if (depth < 0)
    {
        float3 sky = float3(0.5f, 0.7f, 1.0f);
        screenTex[pixel] = tonemapACES(float4(sky, 1.0));
        return;
    }

    // Apply tonemapping
    color = tonemapACES(color);
    screenTex[pixel] = float4(color, 1.0);
}
