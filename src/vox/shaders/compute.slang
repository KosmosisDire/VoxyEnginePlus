#include "atmosphere.slang"
#include "common.slang"

struct TraceResult
{
    bool hit;
    float3 hitPos;
    float3 normal;
    float distance;
    float3 uvs;
};

#define continueDDA(state)   \
    normal = stepDDA(state); \
    steps++;                 \
    continue;

TraceResult traceVoxelRay(float3 origin, float3 direction)
{
    float3 trueStart = origin;
    float3 invDir = inverseDirection(direction);

    TraceResult result;
    result.hit = false;
    result.hitPos = float3(0.0);
    result.normal = float3(0.0);
    result.distance = 0.0;

    // intersect with bounding box
    float tMin, tMax;
    float3 normal;
    bool boxHit = intersectBoxNormal(origin, invDir, float3(0.0), float3(TREE_SIZE), tMin, tMax, normal);
    if (!boxHit)
    {
        return result;
    }

    // adjust origin to be inside the box
    if (tMin > 0.0)
    {
        origin += direction * (tMin + EPSILON);
    }

    int steps = 0;

    uint64_t chunkMask = p.chunk_occupancy_ptr.occupancy[0];

    // Initialize DDA state
    DDAState brickDDA = initDDA(origin, direction, invDir);
    while (steps < MAX_STEPS && isInBounds(brickDDA.cell, int3(TREE_SIZE)))
    {
        int blockIdx = treeLocalIndex(brickDDA.cell);
        if (getBit(chunkMask, blockIdx))
        {
            int blockIdxGlobal = treeGlobalIndex(0, blockIdx);
            uint64_t brickMask = p.brick_occupancy_ptr.occupancy[blockIdxGlobal];

            if (brickMask == 0)
            {
                continueDDA(brickDDA);
            }

            float3 brickUv = getDdaUvs(brickDDA, origin, direction);
            float3 voxelOrigin = clamp(brickUv * TREE_SIZE, float3(EPSILON), float3(TREE_SIZE - EPSILON));

            DDAState voxelDDA = initDDA(voxelOrigin, direction, invDir);
            while (steps < MAX_STEPS && isInBounds(voxelDDA.cell, int3(TREE_SIZE)))
            {
                int voxelIdx = treeLocalIndex(voxelDDA.cell);

                if (getBit(brickMask, voxelIdx))
                {
                    float3 voxelUv = getDdaUvs(voxelDDA, voxelOrigin, direction);
                    int3 absoluteCell = brickDDA.cell * TREE_SIZE + voxelDDA.cell;
                    float3 hitPos = (float3(absoluteCell) + voxelUv) / float(TREE_SIZE);

                    result.hit = true;
                    result.hitPos = hitPos;
                    result.normal = normal;
                    result.distance = length(hitPos - trueStart);
                    result.uvs = voxelUv;

                    return result;
                }

                continueDDA(voxelDDA);
            }
        }

        continueDDA(brickDDA);
    }

    return result;
}

void getRayFromPixel(float2 pixel, uint2 dimensions, CameraData camera, out float3 rayOrigin, out float3 rayDir)
{
    // Convert pixel to NDC space (-1 to 1)
    float2 uv = (pixel + 0.5f) / float2(dimensions);
    float2 ndc = uv * 2.0f - 1.0f;

    // Inverse projection to get view space position
    float4 viewSpace = mul(camera.invViewProj, float4(ndc.x, ndc.y, 1.0f, 1.0f));
    viewSpace /= viewSpace.w;

    // Get ray direction in world space
    rayOrigin = camera.position;
    rayDir = normalize(viewSpace.xyz - camera.position);
}

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

[numthreads(8, 8, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    RWTexture2D<float4> render_image = RWTexture2D<float4>::get(p.image);
    uint2 pixel = pixel_i.xy;
    if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
        return;

    // // Ray setup
    // float3 rayOrigin, rayDir;
    // getRayFromPixel(pixel, p.frame_dim, p.state_ptr.camera, rayOrigin, rayDir);

    // TraceResult trace = traceVoxelRay(rayOrigin, rayDir);

    // // lighting
    // if (trace.hit)
    // {
    //     TraceResult shadowTrace = traceVoxelRay(trace.hitPos + trace.normal * EPSILON, p.state_ptr.sunDir);
    //     float shadow = 1.0;
    //     if (shadowTrace.hit)
    //     {
    //         shadow = 0.0;
    //     }

    //     float3 normal = normalize(trace.normal);
    //     float3 color = float3(0.0);

    //     // simple lambertian shading
    //     float3 sunDir = normalize(p.state_ptr.sunDir);
    //     float lDot = dot(normal, sunDir);
    //     float lightIntensity = max(lDot * shadow, 0.1);

    //     color = getSunColor(sunDir) * lightIntensity;

    //     // apply shading
    //     setPixel(color, pixel, render_image);
    // }
    // else
    // {
    //     // set background color
    //     float3 sky = getAtmosphereColor(rayDir, p.state_ptr.sunDir, p.state_ptr.time);
    //     setPixel(sky, pixel, render_image);
    // }

    // instead set simple gradient color over time
    float3 color = float3(0.0);
    color = lerp(float3(0.0, 0.0, 0.0), float3(0.0, 1.0, 1.0), p.state_ptr.time % 1.0);
    setPixel(color, pixel, render_image);
}
