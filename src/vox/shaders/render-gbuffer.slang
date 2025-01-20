#include "atmosphere.slang"
#include "raycast.slang"
#include "voxel-hashmap.slang"

static const uint SAMPLES_PER_PIXEL = 1;

float3x3 createBasis(float3 normal)
{
    float3 tangent = abs(normal.y) < 0.999 ? normalize(cross(float3(0, 1, 0), normal)) : float3(1, 0, 0);
    float3 bitangent = cross(normal, tangent);
    return float3x3(tangent, normal, bitangent);
}

float3 generateBounceDirection(float3 normal, float2 rand)
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

    return normalize(worldDir);
}

TraceResult computeBounce(float3 origin, float3 normal, float3 blueNoiseSample, out float3 rayDir)
{
    float3 hitPos = origin + normal * EPSILON;

    float2 rand = float2(blueNoiseSample.x, blueNoiseSample.y);

    rayDir = normalize(((blueNoiseSample - 0.5) * 2.0) + normal * 0.9);

    return traceVoxelRay(hitPos, rayDir, 0.0, 256);
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

uint32_t lowerbias32(uint32_t x)
{
    x ^= x >> 16;
    x *= 0xa812d533;
    x ^= x >> 15;
    x *= 0xb278e4ad;
    x ^= x >> 17;
    return x;
}

float3 runMultiSampleBounce(uint2 pixel, float3 rayOrigin, float3 rayDir, RWTexture2D<float4> blueNoiseSample)
{
    float3 bounceColor = float3(0, 0, 0);
    for (uint i = 0; i < SAMPLES_PER_PIXEL; i++)
    {
        float3 bounceRayDir;
        TraceResult bounceResult = computeBounce(rayOrigin, rayDir, blueNoiseSample[(pixel + int2(lowerbias32(i), lowerbias32(i + 100))) % 128].xyz, bounceRayDir);
        bounceColor += calculateBounceColor(bounceResult, bounceRayDir, 1.0);
    }
    return bounceColor / float(SAMPLES_PER_PIXEL);
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

    // blue noise
    RWTexture2D<float4> blueNoise = RWTexture2D<float4>::get(p.blueNoise);

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

    float3 bounceColor = runMultiSampleBounce(pixel, trace.hitPos, trace.normal, blueNoise);

    // write to the gbuffer
    albedo[pixel] = float4(1.0);
    normal[pixel] = float4(trace.normal * 0.5 + 0.5, 1.0);
    position[pixel] = float4(trace.hitPos, 1.0);
    indirect[pixel] = float4(bounceColor, 1.0);
    depth[pixel] = trace.distance;
    voxelIDs[pixel] = int2(globalBrickIndex, trace.voxelIndex);
}
