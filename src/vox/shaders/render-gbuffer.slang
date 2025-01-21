#include "atmosphere.slang"
#include "raycast.slang"
#include "voxel-hashmap.slang"

static const uint SAMPLES_PER_PIXEL = 2;

float3x3 createBasis(float3 normal)
{
    float3 tangent = abs(normal.y) < 0.999 ? normalize(cross(float3(0, 1, 0), normal)) : float3(1, 0, 0);
    float3 bitangent = cross(normal, tangent);
    return float3x3(tangent, normal, bitangent);
}

// Transform blue noise sample with temporal variation
float2 transformBlueNoise(float2 sample, uint frameIndex)
{
    // Simple Cranley-Patterson rotation using frame index
    float2 shift = float2(
        frac(frameIndex * 0.754877669), // Using golden ratio conjugate
        frac(frameIndex * 0.569840291));
    return frac(sample + shift);
}

float3 generateBounceDirection(float3 normal, float3 blueNoiseSample, uint frameIndex)
{
    float3x3 basis = createBasis(normal);

    // Transform the first two components of blue noise
    float2 transformed = transformBlueNoise(blueNoiseSample.xy, frameIndex);

    // Cosine-weighted hemisphere sampling
    float phi = 2.0 * PI * transformed.x;
    float cosTheta = sqrt(transformed.y); // Cosine weighted
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    float3 localDir = float3(
        cos(phi) * sinTheta,
        cosTheta,
        sin(phi) * sinTheta);

    // Transform to world space
    float3 worldDir = mul(basis, localDir);
    return normalize(worldDir);
}

TraceResult computeBounce(float3 origin, float3 normal, float3 blueNoiseSample, uint frameIndex, out float3 rayDir)
{
    float3 hitPos = origin + normal * EPSILON;
    rayDir = generateBounceDirection(normal, blueNoiseSample, frameIndex);
    return traceVoxelRay(hitPos, rayDir, 0.0, 256);
}

float3 calculateBounceColor(TraceResult bounceResult, float3 rayDir)
{
    if (!bounceResult.hit)
    {
        AtmosphereParams atmosphere = initAtmosphere(rayDir, p.state_ptr.sunDir);
        return getBaseSkyColor(atmosphere) * 5;
    }
    else if (brickGlobalIndex(bounceResult.chunkIndex, bounceResult.brickIndex) % 4096 < 64)
    {
        float3 color = float3(0.9, 0.3, 1) * 10;
        float dist = bounceResult.distance;
        float atten = 1.0 / (1.0 + dist * dist * 0.01);
        return color * atten;
    }
    return float3(0, 0, 0);
}

float3 runMultiSampleBounce(uint2 pixel, float3 rayOrigin, float3 rayDir, RWTexture2D<float4> blueNoiseSample)
{
    float3 bounceColor = float3(0, 0, 0);
    uint frameIndex = p.state_ptr.frame;

    for (uint i = 0; i < SAMPLES_PER_PIXEL; i++)
    {
        float3 bounceRayDir;
        float3 noiseSample = blueNoiseSample[pixel % 128].xyz;
        TraceResult bounceResult = computeBounce(rayOrigin, rayDir, noiseSample, frameIndex + i * 32, bounceRayDir);
        bounceColor += calculateBounceColor(bounceResult, bounceRayDir);
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
    RWTexture2D<float3> albedo = RWTexture2D<float3>::get(p.gbuffer.color);
    RWTexture2D<float3> normal = RWTexture2D<float3>::get(p.gbuffer.normal);
    RWTexture2D<float3> position = RWTexture2D<float3>::get(p.gbuffer.position);
    RWTexture2D<float3> indirect = RWTexture2D<float3>::get(p.gbuffer.indirect);
    RWTexture2D<float> depth = RWTexture2D<float>::get(p.gbuffer.depth);
    RWTexture2D<int2> voxelIDs = RWTexture2D<int2>::get(p.gbuffer.voxelIDs);

    Texture2D<float> depthHalfRes = Texture2D<float>::get(p.gbuffer.depthHalfRes);
    RWTexture2D<float4> blueNoise = RWTexture2D<float4>::get(p.blueNoise);

    float3 rayOrigin, rayDir;
    getRayFromPixel(pixel, p.frame_dim, p.state_ptr.camera, rayOrigin, rayDir);

    float startDepth = 0.0;
    if ((bool)p.state_ptr.settings.beamOptimization)
    {
        uint2 prepassPixel = pixel / 2;
        startDepth = depthHalfRes[prepassPixel];

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
    float3 albedoColor = float3(1, 1, 1);

    if (brickGlobalIndex(trace.chunkIndex, trace.brickIndex) % 4096 < 64)
    {
        albedoColor = float3(0.9, 0.3, 1) * 5;
    }

    // write to the gbuffer
    albedo[pixel] = albedoColor;
    normal[pixel] = trace.normal * 0.5 + 0.5;
    position[pixel] = trace.hitPos;
    indirect[pixel] = bounceColor;
    depth[pixel] = trace.distance;
    voxelIDs[pixel] = int2(globalBrickIndex, trace.voxelIndex);
}
