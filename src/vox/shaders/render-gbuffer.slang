#include "atmosphere.slang"
#include "raycast.slang"
#include "voxel-hashmap.slang"

static const float3 UP_VECTOR = float3(0, 1, 0);
static const float3 FORWARD_VECTOR = float3(0, 0, 1);
static const float PARALLEL_THRESHOLD = 0.9999f;
static const uint INIT_RAY_COUNT = 16; // Number of rays to cast for new voxels

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

float3 transformBlueNoise(float3 sample, uint frameIndex)
{
    // Simple Cranley-Patterson rotation using frame index
    float3 shift = float3(
        frac(frameIndex * 0.754877669), // Using golden ratio conjugate
        frac(frameIndex * 0.569840291),
        frac(frameIndex * 0.682327803));
    return frac(sample + shift);
}

TraceResult computeBounce(float3 origin, float3 normal, float3 blueNoiseSample, uint frameIndex, out float3 rayDir)
{
    float3 hitPos = origin + normal * EPSILON;

    float3x3 basis = CreateRotationFromNormal3x3(normal);
    float3 blue = transformBlueNoise(blueNoiseSample, p.state_ptr.frame + uint(origin.x * origin.y * origin.z * 64)) * 2.0 - 1.0;
    blue.y = abs(blue.y);
    blue = mul(basis, blue);
    blue += normal * 0.1;
    rayDir = normalize(blue);

    return traceVoxelRay(hitPos, rayDir, 0, 512);
}

float3 hsv2rgb(float3 hsv)
{
    float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    float3 p = abs(frac(hsv.xxx + K.xyz) * 6.0 - K.www);
    return hsv.z * lerp(K.xxx, saturate(p - K.xxx), hsv.y);
}

float3 getMaterial(TraceResult trace, bool emission)
{
    if (brickGlobalIndex(trace.chunkIndex, trace.brickIndex) % (4096 * 2) < 64)
    {
        // Project position onto a diagonal axis for consistent rainbow direction
        float posProjection = dot(trace.hitPos, normalize(float3(1.0, 1.0, 1.0)));

        // Create scrolling effect with time
        float scrollSpeed = 0.1; // Adjust speed as needed
        float timeOffset = p.state_ptr.time * scrollSpeed;

        // Calculate hue from position and time
        float hue = frac(posProjection * 1 + timeOffset);

        // Convert HSV to RGB with full saturation and value
        float3 color = hsv2rgb(float3(hue, 1.0, 1.0));

        return color * 4;
    }

    return emission ? float3(0.0) : float3(1, 1, 1);
}

float3 calculateBounceColor(TraceResult bounceResult, float3 rayDir)
{
    if (!bounceResult.hit)
    {
        AtmosphereParams atmosphere = initAtmosphere(rayDir, p.state_ptr.sunDir);
        return getBaseSkyColor(atmosphere) * 2;
    }
    else
    {
        return getMaterial(bounceResult, true);
    }
}

bool IsObjectSmallerThanPixel(
    float objectWorldSize,  // Size of object in world units
    float distanceToCamera, // Distance from camera to object in world units
    float horizontalFOV,    // Camera's horizontal FOV in radians
    uint screenWidth)       // Screen width in pixels
{
    // Calculate how many radians one pixel represents
    float radiansPerPixel = horizontalFOV / screenWidth;
    float objectAngularSize = objectWorldSize / distanceToCamera;

    // angular size is smaller than one pixel
    return objectAngularSize < radiansPerPixel;
}

uint get_hashtable_index(uint brickIndex, uint voxelIndex)
{
    uint slot = hashVoxel(brickIndex, voxelIndex);

    uint32_t tries = 0;
    while (tries < 10)
    {
        var brickIdx = p.past_voxel_hashmap_ptr.brickIndices[slot];
        var voxelIdx = p.past_voxel_hashmap_ptr.voxelIndices[slot];

        if (brickIdx == brickIndex && voxelIdx == voxelIndex)
        {
            return slot;
        }
        if (brickIdx == EMPTY_KEY && voxelIdx == EMPTY_KEY)
        {
            return EMPTY_KEY;
        }

        slot = (slot + 1) & (HASH_TABLE_SIZE - 1);

        tries++;
    }

    return EMPTY_KEY;
}

[numthreads(16, 16, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    // return;
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

    RWTexture2D<float> depthHalfRes = RWTexture2D<float>::get(p.gbuffer.depthHalfRes);
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
                startDepth = min(startDepth, depthHalfRes[checkPixel]) - WORLD_VOXEL_SIZE * 0.5;
            }
        }
    }

    TraceResult trace = traceVoxelRay(rayOrigin, rayDir, startDepth);
    int globalBrickIndex = brickGlobalIndex(trace.chunkIndex, trace.brickIndex);

    bool isVoxelFarLOD = IsObjectSmallerThanPixel(
        WORLD_VOXEL_SIZE * 0.15,
        trace.distance,
        p.state_ptr.camera.fov * 0.0174532925f,
        p.frame_dim.x);

    // create id from normal and voxel index
    int voxelIndex = isVoxelFarLOD ? 1 : trace.voxelIndex + 1;

    // int lightRes = trace.distance / 8.0;
    // voxelIndex = trace.voxelCell.x / lightRes * lightRes + (trace.voxelCell.y / lightRes * lightRes << 8) + (trace.voxelCell.z / lightRes * lightRes << 16) + 1;
    // voxelIndex += (int(trace.normal.x * 127) << 0) + (int(trace.normal.y * 127) << 8) + (int(trace.normal.z * 127) << 16);

    // voxelIndex = int(trace.brickUv.x * 16) + (int(trace.brickUv.y * 16) << 8) + (int(trace.brickUv.z * 16) << 16) + 1;

    float3 bounceRayDir;
    float3 bounceColor = float3(0, 0, 0);

    if (all(pixel % 2 == 0))
    {
        float3 baseNoise = blueNoise[(pixel % 64) % 128].xyz;

        // Offset the noise pattern for each additional ray while maintaining good distribution
        float3 offsetNoise = transformBlueNoise(baseNoise, p.state_ptr.frame);

        TraceResult bounceResult = computeBounce(trace.hitPos, trace.normal, offsetNoise, p.state_ptr.frame, bounceRayDir);
        float3 rayBounceColor = calculateBounceColor(bounceResult, bounceRayDir);

        TraceResult shadow = traceVoxelRay(trace.hitPos + trace.normal * EPSILON,
                                           normalize(p.state_ptr.sunDir + (offsetNoise * 2 - 1.0) * 0.004));

        if (!shadow.hit)
        {
            rayBounceColor += getSunColor(initAtmosphere(trace.normal, p.state_ptr.sunDir)) * 0.5;
        }

        if (bounceResult.hit)
        {
            TraceResult bounceShadow = traceVoxelRay(
                bounceResult.hitPos + bounceResult.normal * EPSILON,
                normalize(p.state_ptr.sunDir + (offsetNoise * 2 - 1.0) * 0.004));

            if (!bounceShadow.hit)
            {
                rayBounceColor += getSunColor(initAtmosphere(bounceShadow.normal, p.state_ptr.sunDir)) * 2;
            }
        }

        bounceColor = rayBounceColor;

        uint slot = get_hashtable_index(globalBrickIndex, voxelIndex);
        if (slot == EMPTY_KEY)
        {
            bounceColor = clamp(indirect[pixel / 2], 0, 10);
        }

        indirect[pixel / 2] = bounceColor;
    }

    float3 albedoColor = getMaterial(trace, false);

    // write to the gbuffer
    albedo[pixel] = albedoColor;
    normal[pixel] = trace.normal * 0.5 + 0.5;
    position[pixel] = trace.hitPos;
    depth[pixel] = trace.distance;

    if (trace.hit)
    {
        voxelIDs[pixel] = int2(globalBrickIndex, voxelIndex);
    }
    else
    {
        voxelIDs[pixel] = int2(EMPTY_KEY, EMPTY_KEY);
    }
}
