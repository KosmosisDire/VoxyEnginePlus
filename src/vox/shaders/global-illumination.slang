#include "raycast.slang"
#include "atmosphere.slang"

static const uint SAMPLES_PER_PIXEL = 4;
static const float BOUNCE_ATTENUATION = 1.0;

float wang_hash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return float(seed) * (1.0 / 4294967296.0);
}

// Get adjacent brick index in normal direction
uint getAdjacentBrickIndex(uint brickIndex, float3 normal)
{
    int3 offset = int3(round(normal));
    int3 basePos = brickWorldPos(brickIndex) / WORLD_BRICK_SIZE;
    int3 neighborPos = basePos + offset;

    uint neighborChunkIndex = chunkIndex(neighborPos / CHUNK_SIZE);
    uint neighborLocalIndex = brickLocalIndex(neighborPos % CHUNK_SIZE);
    return brickGlobalIndex(neighborChunkIndex, neighborLocalIndex);
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

    uint seed = pixel_i.x + pixel_i.y * p.frame_dim.x + uint(p.state_ptr.time * 1000.0);
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
        primaryHit.normal
    );

    float3x3 basis = createBasis(primaryHit.normal);
    float3 hitPos = primaryHit.hitPos + primaryHit.normal * EPSILON;

    [unroll]
    for (uint i = 0; i < SAMPLES_PER_PIXEL; i++)
    {
        // Generate random direction in hemisphere
        float2 rand = float2(
            wang_hash(seed + i),
            wang_hash(seed + i + SAMPLES_PER_PIXEL)
        );
        
        float phi = 2.0 * PI * rand.x;
        float theta = acos(sqrt(rand.y));
        float sin_theta = sin(theta);
        
        float3 dir = float3(
            sin_theta * cos(phi),
            cos(theta),
            sin_theta * sin(phi)
        );
        float3 worldDir = mul(basis, dir);

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
            float3 skyColor = getAtmosphereColor(worldDir, p.state_ptr.sunDir, p.state_ptr.time);
            totalBrightness += skyColor;
        }
    }

    float3 finalBrightness = totalBrightness / SAMPLES_PER_PIXEL;

    // Temporal blend
    float3 currentBrightness = getBrickBrightness(storageBrickIndex);
    float3 blendFactor = 0.1;
    float3 newBrightness = lerp(currentBrightness, finalBrightness, blendFactor);

    p.brick_data_ptr.light[storageBrickIndex] = float4(newBrightness, 0.0);
}
