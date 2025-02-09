#include "atmosphere.slang"
#include "lygia/generative/snoise.hlsl"
#include "raycast.slang"
#include "voxel-hashmap.slang"

static const float3 UP_VECTOR = float3(0, 1, 0);
static const float3 FORWARD_VECTOR = float3(0, 0, 1);
static const float PARALLEL_THRESHOLD = 0.9999f;
static const float TEMPORAL_MIN_ALPHA = 0.5;    // Lower = smoother but more ghosting (try 0.01 to 0.1)
static const float TEMPORAL_MOTION_SCALE = 0.5; // Higher = more responsive to motion (try 0.2 to 1.0)
static const float TEMPORAL_CLAMP_SCALE = 0.5;  // Higher = less clamping, more history (try 0.3 to 0.8)

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

float3 sphericalFibonacci(float i, float n)
{
    const float PHI = sqrt(5) * 0.5f + 0.5f;
    float fraction = (i * (PHI - 1)) - floor(i * (PHI - 1));
    float phi = 2.f * PI * fraction;
    float cosTheta = 1.f - (2.f * i + 1.f) * (1.f / n);
    float sinTheta = sqrt(saturate(1.f - cosTheta * cosTheta));

    return float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

TraceResult computeBounce(int2 pixel, float3 origin, float3 normal, float3 blueNoiseSample, uint frameIndex, out float3 rayDir)
{
    float3 hitPos = origin + normal * EPSILON;
    float3x3 basis = CreateRotationFromNormal3x3(normal);
    float3 blue = blueNoiseSample;
    blue = transformBlueNoise(blue, p.state_ptr.frame + uint(pixel.x * p.frame_dim.y + pixel.y));
    blue = blue * 2 - 1;
    blue.y = abs(blue.y);
    blue = mul(basis, blue);
    // blue += normal * 0.05;
    rayDir = normalize(blue);

    // const int num = 1024;
    // rayDir = sphericalFibonacci((frameIndex + blueNoiseSample.x * blueNoiseSample.y * num * 17) % num, num);
    // // rayDir = transformBlueNoise(rayDir, frameIndex + uint(pixel.x * p.frame_dim.y + pixel.y)) * 2.0 - 1.0;
    // rayDir.y = abs(rayDir.y);
    // rayDir = mul(basis, rayDir);

    return traceVoxelRay(hitPos, rayDir, 0, 512);
}

float3 hsv2rgb(float3 hsv)
{
    float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    float3 p = abs(frac(hsv.xxx + K.xyz) * 6.0 - K.www);
    return hsv.z * lerp(K.xxx, saturate(p - K.xxx), hsv.y);
}

Material getMaterial(uint voxelIndex)
{
    return p.materials_ptr.materials[voxelIndex];
}

uint getMaterialId(int2 voxelID)
{
    uint globalVoxelIndex = voxelGlobalIndex(voxelID.x, voxelID.y);
    return (p.voxel_materials_ptr.materials[globalVoxelIndex / 8] >> ((globalVoxelIndex % 8) * 4)) & 0xF;
}

uint getHashtableFaceIndex(TraceResult trace)
{
    bool isVoxelFarLOD = IsObjectSmallerThanPixel(
        WORLD_VOXEL_SIZE * 0.2,
        trace.distance,
        p.state_ptr.camera.fov * 0.0174532925f,
        p.frame_dim.x);

    uint voxelIndex = isVoxelFarLOD ? -1 : ((trace.voxelIndex + 1));
    uint normalIndex = (trace.normal.x != 0 ? 1 : 0) + (trace.normal.y != 0 ? 2 : 0) + (trace.normal.z != 0 ? 4 : 0);
    voxelIndex += 216 * normalIndex;
    return voxelIndex;
}

float3 calculateBounceColor(TraceResult bounceResult, Material hitMat, float3 rayDir)
{
    if (!bounceResult.hit)
    {
        AtmosphereParams atmosphere = initAtmosphere(rayDir, p.state_ptr.sunDir);
        return getBaseSkyColor(atmosphere) * 4;
    }
    else
    {
        TraceResult bounceShadow = traceVoxelRay(bounceResult.hitPos + bounceResult.normal * EPSILON, p.state_ptr.sunDir);

        // get the hashmap entry for the hit voxel and add the light from it if it exists
        float3 passiveBounce = float3(0, 0, 0);
        // uint slot = get_hashtable_index(p.voxel_hashmap_ptr, brickGlobalIndex(bounceResult.chunkIndex, bounceResult.brickIndex), getHashtableFaceIndex(bounceResult));
        // if (slot != EMPTY_KEY)
        // {
        //     var entry = p.voxel_hashmap_ptr.entries[slot];
        //     if (entry.historyCount > 4)
        //     {
        //         passiveBounce = entry.lighting * 0.9;
        //     }
        // }

        if (!bounceShadow.hit)
        {
            return hitMat.emission + getSunColor(initAtmosphere(bounceShadow.normal, p.state_ptr.sunDir)) * 5 * hitMat.albedo;
        }

        return hitMat.emission + passiveBounce;
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

bool validateReprojection(float3 currentPosition, float3 previousPosition)
{
    float xDiff = abs(currentPosition.x - previousPosition.x);
    float yDiff = abs(currentPosition.y - previousPosition.y);
    float zDiff = abs(currentPosition.z - previousPosition.z);

    return xDiff < WORLD_VOXEL_SIZE || yDiff < WORLD_VOXEL_SIZE || zDiff < WORLD_VOXEL_SIZE;
}

float2 calculateMotionClip(float3 worldPos, float4x4 viewProj, float4x4 lastViewProj)
{
    float4 currentClip = mul(viewProj, float4(worldPos, 1.0));
    float4 previousClip = mul(lastViewProj, float4(worldPos, 1.0));

    // Get clip space positions after perspective divide
    float2 currentPos = currentClip.xy / currentClip.w;
    float2 previousPos = previousClip.xy / previousClip.w;

    // Return raw clip space motion
    return previousPos - currentPos;
}

// Get sample position and bilinear weights
void getBilinearWeights(float2 pos, out int2 baseCoord, out float2 weights)
{
    baseCoord = int2(floor(pos));
    weights = frac(pos);
}

// Sample with bilinear interpolation
float4 sampleHistoryBilinear(RWTexture2D<float4> history, float2 pos, float2 dimensions)
{
    int2 baseCoord;
    float2 weights;
    getBilinearWeights(pos, baseCoord, weights);

    // Clamp coordinates to texture bounds
    int2 c00 = clamp(baseCoord, int2(0, 0), int2(dimensions) - 1);
    int2 c10 = clamp(int2(baseCoord.x + 1, baseCoord.y), int2(0, 0), int2(dimensions) - 1);
    int2 c01 = clamp(int2(baseCoord.x, baseCoord.y + 1), int2(0, 0), int2(dimensions) - 1);
    int2 c11 = clamp(baseCoord + int2(1, 1), int2(0, 0), int2(dimensions) - 1);

    // Sample four nearest pixels
    float4 s00 = history[c00];
    float4 s10 = history[c10];
    float4 s01 = history[c01];
    float4 s11 = history[c11];

    // Bilinear blend
    float4 s0 = lerp(s00, s10, weights.x);
    float4 s1 = lerp(s01, s11, weights.x);
    return lerp(s0, s1, weights.y);
}

// Get neighborhood bounds for clamping
void getNeighborhoodBounds(RWTexture2D<float3> currentFrame, int2 pixel, float2 dimensions,
                           out float3 minColor, out float3 maxColor, out float3 avgColor)
{
    minColor = float3(1e6, 1e6, 1e6);
    maxColor = float3(-1e6, -1e6, -1e6);
    avgColor = float3(0, 0, 0);
    float samples = 0;

    // Sample 3x3 neighborhood
    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            int2 samplePos = clamp(pixel + int2(x, y), int2(0, 0), int2(dimensions) - 1);
            float3 sample = currentFrame[samplePos];

            minColor = min(minColor, sample);
            maxColor = max(maxColor, sample);
            avgColor += sample;
            samples += 1.0;
        }
    }

    avgColor /= samples;

    // Add some headroom to the bounds
    float3 delta = (maxColor - minColor) * 0.5;
    minColor -= delta;
    maxColor += delta;
}

[numthreads(16, 16, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    uint2 pixel = pixel_i.xy;
    if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
        return;

    // gbuffer textures
    RWTexture2D<float3> albedo = RWTexture2D<float3>::get(p.gbuffer.color);
    RWTexture2D<int3> normal = RWTexture2D<int3>::get(p.gbuffer.normal);
    RWTexture2D<float3> position = RWTexture2D<float3>::get(p.gbuffer.position);
    RWTexture2D<float2> voxelUVs = RWTexture2D<float2>::get(p.gbuffer.voxelUVs);
    RWTexture2D<float2> brickUVs = RWTexture2D<float2>::get(p.gbuffer.brickUVs);
    RWTexture2D<float4> indirect = RWTexture2D<float4>::get(p.gbuffer.indirect);
    RWTexture2D<float4> indirectLast = RWTexture2D<float4>::get(p.gbuffer.indirectLast);
    RWTexture2D<float2> motion = RWTexture2D<float2>::get(p.gbuffer.motion);
    RWTexture2D<float> depth = RWTexture2D<float>::get(p.gbuffer.depth);
    RWTexture2D<int2> voxelIDs = RWTexture2D<int2>::get(p.gbuffer.voxelIDs);
    RWTexture2D<uint> voxelFaceIDs = RWTexture2D<uint>::get(p.gbuffer.voxelFaceIDs);
    RWTexture2D<uint> materialIDs = RWTexture2D<uint>::get(p.gbuffer.materialIDs);
    RWTexture2D<float> depthHalfRes = RWTexture2D<float>::get(p.gbuffer.depthHalfRes);
    RWTexture2D<float4> blueNoise = RWTexture2D<float4>::get(p.blueNoise);
    RWTexture2D<float> shadowTex = RWTexture2D<float>::get(p.gbuffer.shadow);

    float halfResDepth = depthHalfRes[pixel / 2];

    float3 rayOrigin, rayDir;
    getRayFromPixel(pixel, p.frame_dim, p.state_ptr.camera, rayOrigin, rayDir);

    float startDepth = 0.0;
    if ((bool)p.state_ptr.settings.beamOptimization)
    {
        uint2 prepassPixel = pixel / 2;
        startDepth = halfResDepth;

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
    if (!trace.hit)
    {
        depth[pixel] = 10000.0;
        voxelIDs[pixel] = int2(EMPTY_KEY, EMPTY_KEY);
        return;
    }

    int globalBrickIndex = brickGlobalIndex(trace.chunkIndex, trace.brickIndex);
    uint voxelMaterialId = getMaterialId(int2(globalBrickIndex, trace.voxelIndex));
    Material traceMat = getMaterial(voxelMaterialId);

    // if (dot(trace.normal, rayDir) > (1 / trace.distance * 0.4) - 0.1)
    // {
    //     voxelIndex = 0;
    // }

    float2 motionClip = calculateMotionClip(trace.hitPos, p.state_ptr.camera.viewProj, p.state_ptr.lastCamera.viewProj);
    float2 motionPixels = motionClip * (p.frame_dim * 0.5);
    if (length(motionPixels) < 0.0001 || isnan(motionPixels.x) || isnan(motionPixels.y))
    {
        motionPixels = float2(0, 0);
    }
    float2 prevPos = float2(pixel) + motionPixels;

    // float2 debugMotion = motionPixels;
    // if (pixel.x < 10 && pixel.y < 10)
    // {
    //     // Print debug info for top-left pixels
    //     printf("Pixel: %d,%d Motion: %f,%f\n",
    //            pixel.x, pixel.y,
    //            debugMotion.x, debugMotion.y);
    // }

    float shadow = 1;
    TraceResult bounceShadow = traceVoxelRay(trace.hitPos + trace.normal * EPSILON, normalize(p.state_ptr.sunDir));
    shadow = bounceShadow.hit ? 1 : 0;

    if (all(pixel % 2 == 0))
    {
        float2 halfPixel = pixel / 2;
        float2 dimensions = p.frame_dim / 2;

        float4 blueNoiseSample = blueNoise[halfPixel % 128];
        float3 bounceRayDir;
        TraceResult bounceResult = computeBounce(pixel, trace.hitPos, trace.normal, blueNoiseSample.xyz, p.state_ptr.frame, bounceRayDir);
        Material hitMat = getMaterial(getMaterialId(int2(brickGlobalIndex(bounceResult.chunkIndex, bounceResult.brickIndex), bounceResult.voxelIndex)));
        float3 rayBounceColor = calculateBounceColor(bounceResult, hitMat, bounceRayDir);

        if (bounceResult.hit && all(pixel % 4 == 0))
        {
            float4 blueNoiseSample2 = blueNoise[(halfPixel + 17) % 128];
            TraceResult bounceResult2 = computeBounce(pixel, bounceResult.hitPos, bounceResult.normal, blueNoiseSample2.xyz, p.state_ptr.frame, bounceRayDir);
            Material hitMat2 = getMaterial(getMaterialId(int2(brickGlobalIndex(bounceResult2.chunkIndex, bounceResult2.brickIndex), bounceResult2.voxelIndex)));
            rayBounceColor = lerp(rayBounceColor, calculateBounceColor(bounceResult2, hitMat2, bounceRayDir) * 4, 0.5);
        }

        // Calculate previous position with subpixel precision
        float2 prevPos = prevPos * 0.5;
        // Get previous brick ID from the position
        int2 prevPixel = int2(prevPos);
        int2 prevVoxelData = voxelIDs[prevPixel * 2]; // Multiply by 2 since we're in half-res
        int prevBrickId = prevVoxelData.x;
        // Validate history
        float valid = validateReprojection(position[pixel].xyz, position[prevPixel * 2].xyz);

        prevPos = clamp(prevPos, float2(0), dimensions - 1);

        // Sample lastFrame with bilinear interpolation
        float4 last = sampleHistoryBilinear(indirectLast, prevPos, dimensions);
        last = clamp(last, float4(0), float4(10000));

        if (!valid)
        {
            last.w = 0;
        }

        // do extra bounces for new pixels
        if (last.w < 2)
        {
            float4 blueNoiseSample3 = blueNoise[(halfPixel + 46) % 128];
            TraceResult bounceResult3 = computeBounce(pixel, trace.hitPos, trace.normal, blueNoiseSample3.xyz, p.state_ptr.frame, bounceRayDir);
            Material hitMat3 = getMaterial(getMaterialId(int2(brickGlobalIndex(bounceResult3.chunkIndex, bounceResult3.brickIndex), bounceResult3.voxelIndex)));
            rayBounceColor = lerp(rayBounceColor, calculateBounceColor(bounceResult3, hitMat3, bounceRayDir), 0.5);
        }

        float blendFactor = (1 / (last.w + 1));
        blendFactor = clamp(blendFactor, TEMPORAL_MIN_ALPHA, 1.0);

        // Final blend
        float3 blended = lerp(last.rgb, rayBounceColor, blendFactor);
        indirect[halfPixel] = float4(blended, last.w + 1);
    }

    float3 albedoColor = traceMat.albedo + traceMat.emission;

    float2 voxelFaceUV = trace.normal.x != 0 ? trace.voxelUv.yz : trace.normal.y != 0 ? trace.voxelUv.xz
                                                                                      : trace.voxelUv.xy;
    float2 brickFaceUV = trace.normal.x != 0 ? trace.brickUv.yz : trace.normal.y != 0 ? trace.brickUv.xz
                                                                                      : trace.brickUv.xy;

    // write to the gbuffer
    albedo[pixel] = albedoColor;
    normal[pixel] = int3(normalize(trace.normal));
    position[pixel] = trace.hitPos;
    voxelUVs[pixel] = voxelFaceUV;
    brickUVs[pixel] = brickFaceUV;
    depth[pixel] = trace.distance;
    motion[pixel] = trace.hit ? motionPixels : float2(0, 0);
    shadowTex[pixel] = 1 - shadow;
    materialIDs[pixel] = voxelMaterialId;

    if (trace.hit)
    {
        voxelIDs[pixel] = int2(globalBrickIndex, trace.voxelIndex);
        voxelFaceIDs[pixel] = getHashtableFaceIndex(trace);
    }
    else
    {
        voxelIDs[pixel] = int2(EMPTY_KEY, EMPTY_KEY);
        voxelFaceIDs[pixel] = EMPTY_KEY;
    }
}
