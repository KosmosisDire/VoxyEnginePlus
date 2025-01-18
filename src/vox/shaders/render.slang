#include "atmosphere.slang"
#include "common.slang"

struct TraceResult
{
    bool hit;
    float3 hitPos;
    float3 normal;
    float distance;
    float3 uvs;
    int steps;
    int chunkIndex;
    int brickIndex;
};

#define continueDDA(state)   \
    normal = stepDDA(state); \
    result.steps++;          \
    continue;

TraceResult traceVoxelRay(float3 origin, float3 direction, int maxSteps = MAX_STEPS)
{
    float3 trueStart = origin;
    float3 invDir = inverseDirection(direction);

    TraceResult result;
    result.hit = false;
    result.hitPos = float3(0.0);
    result.normal = float3(0.0);
    result.distance = 0.0;
    result.uvs = float3(0.0);
    result.steps = 0;

    // intersect with bounding box
    float tMin, tMax;
    float3 normal;
    bool boxHit = intersectBoxNormal(origin, invDir, float3(0.0), float3(GRID_SIZE), tMin, tMax, normal);
    if (!boxHit)
    {
        return result;
    }

    // adjust origin to be inside the box
    if (tMin > EPSILON)
    {
        origin += direction * (tMin + EPSILON);
    }

    DDAState chunkDDA = initDDA(origin, direction, invDir);
    while (result.steps < maxSteps && isInBounds(chunkDDA.cell, int3(GRID_SIZE)))
    {
        int chunkIdx = chunkIndex(chunkDDA.cell);
        daxa_u64 chunkMask = p.chunk_occupancy_ptr.occupancy[chunkIdx];

        if (chunkMask == 0)
        {
            continueDDA(chunkDDA);
        }

        float3 chunkUv = getDdaUvs(chunkDDA, origin, direction);

        float3 brickOrigin = clamp(chunkUv * CHUNK_SIZE, float3(EPSILON), float3(CHUNK_SIZE - EPSILON));
        DDAState brickDDA = initDDA(brickOrigin, direction, invDir);
        while (result.steps < maxSteps && isInBounds(brickDDA.cell, int3(CHUNK_SIZE)))
        {
            int brickIdx = brickLocalIndex(brickDDA.cell);
            if (getBit(chunkMask, brickIdx))
            {
                int brickIdxGlobal = brickGlobalIndex(chunkIdx, brickIdx);
                BrickBitmask brickMask = p.brick_occupancy_ptr.occupancy[brickIdxGlobal];

                if (brickBitmaskEmpty(brickMask))
                {
                    continueDDA(brickDDA);
                }

                float3 brickUv = getDdaUvs(brickDDA, brickOrigin, direction);

                float3 voxelOrigin = clamp(brickUv * BRICK_SIZE, float3(EPSILON), float3(BRICK_SIZE - EPSILON));
                DDAState voxelDDA = initDDA(voxelOrigin, direction, invDir);
                while (result.steps < maxSteps && isInBounds(voxelDDA.cell, int3(BRICK_SIZE)))
                {
                    int voxelIdx = voxelLocalIndex(voxelDDA.cell);

                    if (getBit(brickMask, voxelIdx))
                    {
                        float3 voxelUv = getDdaUvs(voxelDDA, voxelOrigin, direction);
                        float3 hitPos = chunkDDA.cell + brickDDA.cell / float(CHUNK_SIZE) + voxelDDA.cell / float(BRICK_SIZE) / float(CHUNK_SIZE) + voxelUv / float(BRICK_SIZE) / float(CHUNK_SIZE);

                        result.hit = true;
                        result.hitPos = hitPos;
                        result.normal = normal;
                        result.distance = length(hitPos - trueStart);
                        result.uvs = voxelUv;
                        result.chunkIndex = chunkIdx;
                        result.brickIndex = brickIdx;

                        // Mark brick as visible
                        uint global_idx = chunkIdx * CHUNK_SIZE_CUBE + brickIdx;
                        uint bit_idx = global_idx % 32;
                        uint array_idx = global_idx / 32;
                        InterlockedOr(p.visible_bricks_ptr.bits[array_idx], 1u << bit_idx);

                        return result;
                    }

                    continueDDA(voxelDDA);
                }
            }

            continueDDA(brickDDA);
        }

        continueDDA(chunkDDA);
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
    RWTexture2D<float4> render_image = RWTexture2D<float4>::get(p.image);
    uint2 pixel = pixel_i.xy;
    if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
        return;

    // Ray setup
    float3 rayOrigin, rayDir;
    getRayFromPixel(pixel, p.frame_dim, p.state_ptr.camera, rayOrigin, rayDir);

    TraceResult trace = traceVoxelRay(rayOrigin, rayDir);
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

        // Modify lighting calculation to include AO
        // Check if this brick is in the visible set
        uint global_brick_idx = trace.chunkIndex * CHUNK_SIZE_CUBE + trace.brickIndex;
        bool is_visible = false;
        
        for(uint i = 0; i < p.compact_visible_ptr.count; i++) {
            VisibleBrick brick = p.compact_visible_ptr.bricks[i];
            if(brick.chunk_index == trace.chunkIndex && brick.brick_index == trace.brickIndex) {
                is_visible = true;
                break;
            }
        }

        float3 directLight = (sunColor * 0.7 + normalSkyLight * 0.3) * lDot * shadow * 1.0;
        float3 ambientLight = normalSkyLight * 0.5;
        color = directLight + ambientLight * p.state_ptr.settings.skyLight;
        
        // Highlight visible bricks
        if(is_visible) {
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
    if ((bool)p.state_ptr.settings.showUvs)
        color = trace.hitPos / float(GRID_SIZE);

    setPixel(color, pixel, render_image);
}
