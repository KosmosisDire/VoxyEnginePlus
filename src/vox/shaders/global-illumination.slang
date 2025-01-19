#include "atmosphere.slang"
#include "raycast.slang"

static const uint SAMPLES_PER_PIXEL = 1;

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

float3 generateBounceDirection(float3 normal, float2 rand, float sunBias)
{
    float3x3 basis = createBasis(normal);

    float phi = 2.0 * PI * rand.x;
    float theta = acos(sqrt(rand.y));
    float sin_theta = sin(theta);

    float3 dir = float3(
        sin_theta * cos(phi),
        cos(theta),
        sin_theta * sin(phi));
    float3 worldDir = mul(basis, dir);

    return normalize(worldDir + p.state_ptr.sunDir * sunBias);
}

TraceResult computeBounce(float3 origin, float3 normal, uint seed, uint sampleOffset, float sunBias)
{
    float3 hitPos = origin + normal * EPSILON;

    float2 rand = float2(
        wang_hash(seed + sampleOffset),
        wang_hash(seed + sampleOffset + SAMPLES_PER_PIXEL));

    float3 worldDir = generateBounceDirection(normal, rand, sunBias);

    return traceVoxelRay(hitPos, worldDir);
}

float3 calculateBounceColor(TraceResult bounceResult, float3 rayDir, float intensity)
{
    if (!bounceResult.hit)
    {
        AtmosphereParams atmosphere = initAtmosphere(rayDir, p.state_ptr.sunDir);
        return getBaseSkyColor(atmosphere) * intensity;
    }
    return float3(0, 0, 0);
}

[numthreads(8, 8, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    if (pixel_i.x >= p.frame_dim.x || pixel_i.y >= p.frame_dim.y)
        return;

    uint seed = pixel_i.x * pixel_i.y * p.frame_dim.x * uint(p.state_ptr.time * 123.456);
    float3 accumulatedColor = 0.0;

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

    if (storageBrickIndex < 0 || storageBrickIndex >= GRID_SIZE_CUBE * CHUNK_SIZE_CUBE)
        return;

    uint sampleCount;
    // if we are far use 2 samples, otherwise use 1
    if (length(primaryHit.hitPos - rayOrigin) > 10.0)
        sampleCount = 5;
    else
        sampleCount = 1;

    for (uint i = 0; i < sampleCount; i++)
    {
        TraceResult primaryBounce = computeBounce(primaryHit.hitPos, primaryHit.normal, seed, i, 1.0);
        float3 primaryBounceColor = calculateBounceColor(primaryBounce, primaryHit.normal, 1.0);
        accumulatedColor += primaryBounceColor;

        // If primary bounce hit something, compute secondary bounce
        if (primaryBounce.hit)
        {
            TraceResult secondaryBounce = computeBounce(
                primaryBounce.hitPos,
                primaryBounce.normal,
                seed,
                i + sampleCount * 2,
                0.5);

            accumulatedColor += calculateBounceColor(secondaryBounce, primaryBounce.normal, 0.5);
        }
    }

    accumulatedColor /= sampleCount;

    InterlockedAdd(p.brick_data_ptr.light[storageBrickIndex].x, accumulatedColor.x);
    InterlockedAdd(p.brick_data_ptr.light[storageBrickIndex].y, accumulatedColor.y);
    InterlockedAdd(p.brick_data_ptr.light[storageBrickIndex].z, accumulatedColor.z);
    InterlockedAdd(p.brick_data_ptr.rayCount[storageBrickIndex], 1);
}
