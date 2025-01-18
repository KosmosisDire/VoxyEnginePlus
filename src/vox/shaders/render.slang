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
    RWTexture2D<float4> render_image = RWTexture2D<float4>::get(p.final_image);
    RWTexture2D<float> dpeth_prepass = RWTexture2D<float>::get(p.depth_prepass);
    uint2 pixel = pixel_i.xy;
    if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
        return;

    // Ray setup
    float3 rayOrigin, rayDir;
    getRayFromPixel(pixel, p.frame_dim, p.state_ptr.camera, rayOrigin, rayDir);

    // Beam optimization
    float startDepth = 0.0;
    if ((bool)p.state_ptr.settings.beamOptimization)
    {
        // check center and 4 adjacent pixels, use minimum depth
        uint2 prepassPixel = pixel / 2;
        float startDepth = dpeth_prepass[prepassPixel];

        // Check adjacent pixels (up, down, left, right)
        if (prepassPixel.y > 0)
            startDepth = min(startDepth, dpeth_prepass[uint2(prepassPixel.x, prepassPixel.y - 1)]);
        if (prepassPixel.y < p.frame_dim.y / 2 - 1)
            startDepth = min(startDepth, dpeth_prepass[uint2(prepassPixel.x, prepassPixel.y + 1)]);
        if (prepassPixel.x > 0)
            startDepth = min(startDepth, dpeth_prepass[uint2(prepassPixel.x - 1, prepassPixel.y)]);
        if (prepassPixel.x < p.frame_dim.x / 2 - 1)
            startDepth = min(startDepth, dpeth_prepass[uint2(prepassPixel.x + 1, prepassPixel.y)]);
    }

    TraceResult trace = traceVoxelRay(rayOrigin, rayDir, startDepth);
    float3 color = float3(0.0);

    // lighting
    if (trace.hit)
    {
        TraceResult shadowTrace = traceVoxelRay(trace.hitPos + trace.normal * EPSILON, p.state_ptr.sunDir);
        float shadow = 1.0;
        if (shadowTrace.hit)
        {
            shadow = 0.2;
        }

        float3 normal = normalize(trace.normal);

        // simple lambertian shading with AO
        float3 sunDir = normalize(p.state_ptr.sunDir);
        float lDot = dot(normal, sunDir);

        float3 normalSkyLight = getBaseSkyColor(initAtmosphere(normal, p.state_ptr.sunDir));
        float3 sunColor = getSunColor(initAtmosphere(rayDir, p.state_ptr.sunDir));

        // Check if this brick is in the visible set
        uint global_brick_idx = trace.chunkIndex * CHUNK_SIZE_CUBE + trace.brickIndex;
        bool is_visible = false;

        uint bit_idx = global_brick_idx % 32;
        uint array_idx = global_brick_idx / 32;

        if ((p.visible_bricks_ptr.bits[array_idx] & (1u << bit_idx)) != 0)
        {
            is_visible = true;
        }

        float3 directLight = (sunColor * 0.7 + normalSkyLight * 0.3) * lDot * shadow * 1.0;
        float3 ambientLight = normalSkyLight * 0.5;
        color = directLight + ambientLight * p.state_ptr.settings.skyLight;

        // Highlight visible bricks
        if (is_visible)
        {
            color += float3(0.3, 0.0, 0.0);
        }

        // darken sides of the voxel
        float up = dot(normal, float3(0, 1, 0));
        if (up < 0.1)
        {
            color *= 0.8;
        }

        // apply tone mapping
        color = ACESFilm(color);
    }
    else
    {
        // set background color
        color = getAtmosphereColor(rayDir, p.state_ptr.sunDir, p.state_ptr.time);
    }

    if ((bool)p.state_ptr.settings.showSteps)
        color = float3(trace.steps / 200.0);
    if ((bool)p.state_ptr.settings.showNormals)
        color = trace.normal;
    if ((bool)p.state_ptr.settings.showHitPos)
        color = trace.hitPos / float(GRID_SIZE);
    if ((bool)p.state_ptr.settings.showUVs)
        color = trace.uvs;
    if ((bool)p.state_ptr.settings.showDepth)
        color = float3(trace.distance / 100.0);
    if ((bool)p.state_ptr.settings.showDepthPrepass)
        color = float3(dpeth_prepass[pixel / 2] / 100.0);

    setPixel(color, pixel, render_image);
}
