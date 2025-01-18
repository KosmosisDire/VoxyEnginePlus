#include "atmosphere.slang"
#include "raycast.slang"

static const uint SAMPLES_PER_PIXEL = 1;
static const float BOUNCE_ATTENUATION = 0.0;

float wang_hash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return float(seed) * (1.0 / 4294967296.0);
}

float3x3 createBasis(float3 normal)
{
    float3 tangent = abs(normal.y) < 0.999 ? normalize(cross(float3(0, 1, 0), normal)) : float3(1, 0, 0);
    float3 bitangent = cross(normal, tangent);
    return float3x3(tangent, normal, bitangent);
}

// Safe brick data access
float3 getBrickBrightness(uint index)
{
    return p.brick_data_ptr.light[index].xyz;
}

[numthreads(8, 8, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    if (pixel_i.x >= p.frame_dim.x || pixel_i.y >= p.frame_dim.y)
        return;

    uint seed = pixel_i.x * pixel_i.y * p.frame_dim.x * uint(p.state_ptr.time * 123.456);
    float3 totalBrightness = 0.0;

    // Get primary ray hit information
    float3 rayOrigin, rayDir;
    getRayFromPixel(pixel_i.xy, p.frame_dim, p.state_ptr.camera, rayOrigin, rayDir);
    TraceResult primaryHit = traceVoxelRay(rayOrigin, rayDir);

    if (!primaryHit.hit)
        return;

    // Get the brick that will store the light (adjacent in normal direction)
    uint storageBrickIndex = getAdjacentBrickIndex(
        brickGlobalIndex(primaryHit.chunkIndex, primaryHit.brickIndex),
        primaryHit.normal);

    float3x3 basis = createBasis(primaryHit.normal);
    float3 hitPos = primaryHit.hitPos + primaryHit.normal * EPSILON;

    [unroll]
    for (uint i = 0; i < SAMPLES_PER_PIXEL; i++)
    {
        // Generate random direction in hemisphere
        float2 rand = float2(
            wang_hash(seed + i),
            wang_hash(seed + i + SAMPLES_PER_PIXEL));

        float phi = 2.0 * PI * rand.x;
        float theta = acos(sqrt(rand.y));
        float sin_theta = sin(theta);

        float3 dir = float3(
            sin_theta * cos(phi),
            cos(theta),
            sin_theta * sin(phi));
        float3 worldDir = mul(basis, dir);

        worldDir = normalize(worldDir + p.state_ptr.sunDir * 1);

        TraceResult bounceHit = traceVoxelRay(hitPos, worldDir);

        if (bounceHit.hit)
        {
            uint bounceBrickIndex = brickGlobalIndex(bounceHit.chunkIndex, bounceHit.brickIndex);
            float3 incomingLight = getBrickBrightness(bounceBrickIndex);
            float cosTerm = abs(dot(worldDir, bounceHit.normal));
            totalBrightness += incomingLight * cosTerm * BOUNCE_ATTENUATION;
        }
        else
        {
            // Sample sky color
            AtmosphereParams atmosphere = initAtmosphere(worldDir, p.state_ptr.sunDir);
            float3 skyColor = getBaseSkyColor(atmosphere);
            totalBrightness += skyColor;
        }
    }

    float3 finalBrightness = totalBrightness / SAMPLES_PER_PIXEL;

    // Use atomic operations to accumulate light values and sample count
    DeviceMemoryBarrier();
    InterlockedAdd(p.brick_data_ptr.light[storageBrickIndex].x, finalBrightness.x);
    InterlockedAdd(p.brick_data_ptr.light[storageBrickIndex].y, finalBrightness.y);
    InterlockedAdd(p.brick_data_ptr.light[storageBrickIndex].z, finalBrightness.z);
    InterlockedAdd(p.brick_data_ptr.light[storageBrickIndex].w, 1);
}
