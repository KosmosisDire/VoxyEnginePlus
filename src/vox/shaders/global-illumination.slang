#include "raycast.slang"

static const uint N_PHI = 8;
static const uint N_THETA = 4;
static const uint TOTAL_SAMPLES = N_PHI * N_THETA;
static const float BOUNCE_ATTENUATION = 1.0;

float rand(float2 seed)
{
    return frac(sin(dot(seed, float2(12.9898, 78.233))) * 43758.5453);
}

float3x3 createBasis(float3 normal)
{
    float3 tangent = abs(normal.y) < 0.999 ? normalize(cross(float3(0, 1, 0), normal)) : float3(1, 0, 0);
    float3 bitangent = cross(normal, tangent);
    return float3x3(tangent, normal, bitangent);
}

float3 sphericalToCartesian(float phi, float theta, float3x3 basis)
{
    float sin_theta = sin(theta);
    float3 dir = float3(
        sin_theta * cos(phi),
        cos(theta),
        sin_theta * sin(phi));
    return mul(basis, dir);
}

float2 hammersley(uint i, uint N)
{
    float2 p;
    p.x = float(i) / float(N);
    uint bits = i;
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    p.y = float(bits) * 2.3283064365386963e-10;
    return p;
}

float wang_hash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return float(seed) * (1.0 / 4294967296.0);
}

// Safe brick data access
float3 getBrickBrightness(uint index)
{
    return p.brick_data_ptr.light[index].xyz;
}

// Get brick index from position
uint getBrickIndexAtOffset(uint baseBrickIndex, int3 offset)
{
    int3 basePos = brickWorldPos(baseBrickIndex) / WORLD_BRICK_SIZE;
    int3 neighborPos = basePos + offset;

    uint neighborChunkIndex = chunkIndex(neighborPos / CHUNK_SIZE);
    uint neighborLocalIndex = brickLocalIndex(neighborPos % CHUNK_SIZE);
    return brickGlobalIndex(neighborChunkIndex, neighborLocalIndex);
}

[numthreads(1024, 1, 1)]
void main(uint3 brick_i: SV_DispatchThreadID)
{
    uint brickIndex = p.compact_visible_ptr.indices[brick_i.x];

    float3x3 sunBasis = createBasis(normalize(p.state_ptr.sunDir));
    float3 worldBrickPos = brickWorldPos(brickIndex) + float3(0.5, 0.5, 0.5) * WORLD_BRICK_SIZE;
    float3 rayOrigin = worldBrickPos + p.state_ptr.sunDir * WORLD_BRICK_SIZE * 0.5;

    float3 totalBrightness = 0.0;
    uint seed = brick_i.x + uint(p.state_ptr.time * 1000.0);

    [unroll]
    for (uint i = 0; i < TOTAL_SAMPLES; i++)
    {
        float2 h = hammersley(i, TOTAL_SAMPLES);
        h = frac(h + float2(wang_hash(seed + i), wang_hash(seed + i + TOTAL_SAMPLES)));

        float phi = 2.0 * PI * h.x;
        float theta = acos(1.0 - h.y);

        float sin_theta = sin(theta);
        float3 dir = float3(
            sin_theta * cos(phi),
            cos(theta),
            sin_theta * sin(phi));

        float3 rayDir = normalize(mul(sunBasis, dir) + p.state_ptr.sunDir * 5);

        float3 jitter = float3(
                            wang_hash(seed + i),
                            wang_hash(seed + i + TOTAL_SAMPLES),
                            wang_hash(seed + i + TOTAL_SAMPLES * 2)) *
                        0.1;
        float3 jitteredRayOrigin = rayOrigin + jitter;

        TraceResult trace = traceVoxelRay(jitteredRayOrigin, rayDir, 0.5, 64);

        if (trace.hit)
        {
            float3 hitPos = trace.hitPos - rayDir * EPSILON;
            int3 brickPos = int3(floor(hitPos / WORLD_BRICK_SIZE));
            uint hitChunkIndex = chunkIndex(brickPos / CHUNK_SIZE);
            uint hitLocalBrickIndex = brickLocalIndex(brickPos % CHUNK_SIZE);
            uint hitBrickIndex = brickGlobalIndex(hitChunkIndex, hitLocalBrickIndex);

            if (hitBrickIndex < CHUNK_SIZE_CUBE * GRID_SIZE_CUBE)
            {
                float3 hitBrightness = getBrickBrightness(hitBrickIndex);
                float cosTerm = abs(dot(rayDir, trace.normal));
                totalBrightness += hitBrightness * cosTerm * BOUNCE_ATTENUATION;
            }
        }
        else
        {
            // dot with sun
            float cosTerm = dot(rayDir, p.state_ptr.sunDir) * 0.5 + 0.5;
            totalBrightness += 1.0 * cosTerm;
        }
    }

    float3 finalBrightness = totalBrightness / TOTAL_SAMPLES;

    // Temporal blend
    float3 currentBrightness = getBrickBrightness(brickIndex);
    float3 blendFactor = 0.1;
    float3 newBrightness = lerp(currentBrightness, finalBrightness, blendFactor);

    p.brick_data_ptr.light[brickIndex] = float4(newBrightness, 0.0);
}
