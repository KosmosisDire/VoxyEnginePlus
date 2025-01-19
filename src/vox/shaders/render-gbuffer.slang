#include "atmosphere.slang"
#include "raycast.slang"

float4 alphaBlend(float4 src, float4 dst)
{
    return src + dst * (1.0 - src.w);
}

void setPixel(float4 color, uint2 pixel, RWTexture2D<float4> render_image)
{
    if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
        return;

    render_image[pixel] = alphaBlend(color, render_image[pixel]);
}

void setPixel(float3 color, uint2 pixel, RWTexture2D<float4> render_image)
{
    if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
        return;

    render_image[pixel] = float4(color, 1.0);
}

float3 ACESFilm(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

[numthreads(8, 8, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    uint2 pixel = pixel_i.xy;
    if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
        return;

    // gbuffer textures
    RWTexture2D<float4> albedo = RWTexture2D<float4>::get(p.gbuffer.color);
    RWTexture2D<float4> normal = RWTexture2D<float4>::get(p.gbuffer.normal);
    RWTexture2D<float4> position = RWTexture2D<float4>::get(p.gbuffer.position);
    RWTexture2D<float4> indirect = RWTexture2D<float4>::get(p.gbuffer.indirect);
    RWTexture2D<float> depth = RWTexture2D<float>::get(p.gbuffer.depth);
    RWTexture2D<int2> voxelIDs = RWTexture2D<int2>::get(p.gbuffer.voxelIDs);

    // depth half res
    Texture2D<float> depthHalfRes = Texture2D<float>::get(p.gbuffer.depthHalfRes);

    // Ray setup
    float3 rayOrigin, rayDir;
    getRayFromPixel(pixel, p.frame_dim, p.state_ptr.camera, rayOrigin, rayDir);

    // Beam optimization
    float startDepth = 0.0;
    if ((bool)p.state_ptr.settings.beamOptimization)
    {
        uint2 prepassPixel = pixel / 2;
        startDepth = depthHalfRes[prepassPixel];

        // Check all pixels in a 3x3 grid
        for (int dy = -1; dy <= 1; dy++)
        {
            for (int dx = -1; dx <= 1; dx++)
            {
                uint2 checkPixel = uint2(
                    clamp(int(prepassPixel.x) + dx, 0, int(p.frame_dim.x / 2) - 1),
                    clamp(int(prepassPixel.y) + dy, 0, int(p.frame_dim.y / 2) - 1));
                startDepth = min(startDepth, depthHalfRes[checkPixel]);
            }
        }
    }

    TraceResult trace = traceVoxelRay(rayOrigin, rayDir, startDepth);

    int globalBrickIndex = brickGlobalIndex(trace.chunkIndex, trace.brickIndex);

    // write to the gbuffer
    albedo[pixel] = float4(1.0);
    normal[pixel] = float4(trace.normal * 0.5 + 0.5, 1.0);
    position[pixel] = float4(trace.hitPos, 1.0);
    indirect[pixel] = float4(0.0);
    depth[pixel] = trace.distance;
    voxelIDs[pixel] = int2(globalBrickIndex, trace.voxelIndex);
}
