#include "atmosphere.slang"
#include "raycast.slang"
#include "voxel-hashmap.slang"

static const float3 UP_VECTOR = float3(0, 1, 0);
static const float3 FORWARD_VECTOR = float3(0, 0, 1);
static const float PARALLEL_THRESHOLD = 0.9999f;

// Creates a 3x3 rotation matrix that rotates from up vector to the given normal
float3x3 CreateRotationFromNormal3x3(float3 normal)
{
    // Ensure normal is normalized
    normal = normalize(normal);

    // Check if normal is nearly parallel to up vector
    float upDot = dot(normal, UP_VECTOR);

    float3 right;
    if (abs(upDot) > PARALLEL_THRESHOLD)
    {
        // If parallel, use forward vector as reference
        right = normalize(cross(FORWARD_VECTOR, normal));
    }
    else
    {
        right = normalize(cross(UP_VECTOR, normal));
    }

    // Create forward vector to complete orthonormal basis
    float3 forward = normalize(cross(normal, right));

    // Construct rotation matrix
    // Each column represents one of our basis vectors
    return float3x3(
        right.x, normal.x, forward.x,
        right.y, normal.y, forward.y,
        right.z, normal.z, forward.z);
}

TraceResult computeBounce(float3 origin, float3 normal, float3 blueNoiseSample, uint frameIndex, out float3 rayDir)
{
    float3 hitPos = origin + normal * EPSILON;

    float3x3 basis = CreateRotationFromNormal3x3(normal);
    float3 blue = (blueNoiseSample * 2.0 - 1.0);
    blue.y = abs(blue.y);
    blue = mul(basis, blue);
    blue += normal * 0.1;
    rayDir = normalize(blue);
    return traceVoxelRay(hitPos, rayDir, 0.0, 64);
}

float3 calculateBounceColor(TraceResult bounceResult, float3 rayDir)
{
    if (!bounceResult.hit)
    {
        AtmosphereParams atmosphere = initAtmosphere(rayDir, p.state_ptr.sunDir);
        return getBaseSkyColor(atmosphere) * 3;
    }
    else if (brickGlobalIndex(bounceResult.chunkIndex, bounceResult.brickIndex) % 4096 < 64)
    {
        float3 color = float3(0.9, 0.3, 1) * 1;
        return color;
    }

    return -float3(0.01);
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

    float3 bounceRayDir;
    float3 bounceColor = float3(0, 0, 0);

    float2 sampleUv = pixel / float2(p.frame_dim) * 128;
    sampleUv = pixel;
    // sampleUv = trace.uvs.xy * 128;
    // sampleUv = ((trace.hitPos.xy + trace.hitPos.yz * 17 + trace.hitPos.zx * 23) * 160.0);
    TraceResult bounceResult = computeBounce(trace.hitPos, trace.normal, blueNoise[sampleUv % 128].xyz, 0, bounceRayDir);
    bounceColor += calculateBounceColor(bounceResult, bounceRayDir);

    float3 albedoColor = float3(1, 1, 1);

    if (brickGlobalIndex(trace.chunkIndex, trace.brickIndex) % 4096 < 64)
    {
        albedoColor = float3(0.9, 0.3, 1) * 10;
    }

    float3 indirectFinal = bounceColor;

    // write to the gbuffer
    albedo[pixel] = albedoColor;
    normal[pixel] = trace.normal * 0.5 + 0.5;
    position[pixel] = trace.hitPos;
    indirect[pixel] = indirectFinal;
    depth[pixel] = trace.distance;
    voxelIDs[pixel] = int2(globalBrickIndex, trace.voxelIndex);
}
