#include "atmosphere.slang"
#include "lygia/generative/snoise.hlsl"
#include "raycast.slang"
#include "voxel-hashmap.slang"

static const float3 UP_VECTOR = float3(0, 1, 0);
static const float3 FORWARD_VECTOR = float3(0, 0, 1);
static const float PARALLEL_THRESHOLD = 0.9999f;
static const float TEMPORAL_MIN_ALPHA = 0.005;  // Lower = smoother but more ghosting (try 0.01 to 0.1)
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
    if (trace.absCell.x % 47 == 0)
    {
        // Project position onto a diagonal axis for consistent rainbow direction
        float posProjection = dot(trace.hitPos, normalize(float3(1.0, 1.0, 1.0)));

        // Create scrolling effect with time
        float scrollSpeed = 0.03; // Adjust speed as needed
        float timeOffset = p.state_ptr.time * scrollSpeed;

        // Calculate hue from position and time
        float hue = frac(posProjection * 0.3 + timeOffset);

        // Convert HSV to RGB with full saturation and value
        float3 color = hsv2rgb(float3(hue, 1.0, 1.0));

        return color * 5;
    }

    return emission ? float3(0.0) : (float3(0.17f, 0.33f, 0.23f) + snoise(trace.absCell / 4.0) * 0.01);
}

float3 calculateBounceColor(TraceResult bounceResult, float3 rayDir)
{
    if (!bounceResult.hit)
    {
        AtmosphereParams atmosphere = initAtmosphere(rayDir, p.state_ptr.sunDir);
        return getBaseSkyColor(atmosphere) * 3;
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

bool isEffectivelyZero(float3 color)
{
    return length(color) < EPSILON;
}

float3 sampleNeighborhood(RWTexture2D<float3> indirect, uint2 pixel, out float3 minColor, out float3 maxColor)
{
    float3 average = 0.0;
    minColor = float3(MAX_FLOAT);
    maxColor = float3(MIN_FLOAT);
    int samples = 0;

    // Sample 3x3 neighborhood
    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            int2 samplePos = int2(pixel) + int2(x, y);

            // Check bounds
            if (samplePos.x >= 0 && samplePos.y >= 0 &&
                samplePos.x < p.frame_dim.x / 2 && samplePos.y < p.frame_dim.y / 2)
            {
                float3 sample = indirect[samplePos];
                if (!isEffectivelyZero(sample))
                {
                    average += sample;
                    minColor = min(minColor, sample);
                    maxColor = max(maxColor, sample);
                    samples++;
                }
            }
        }
    }

    return samples > 0 ? average / samples : float3(0, 0, 0);
}

bool validateReprojection(float3 currentPosition, float3 previousPosition)
{
    float xDiff = abs(currentPosition.x - previousPosition.x);
    float yDiff = abs(currentPosition.y - previousPosition.y);
    float zDiff = abs(currentPosition.z - previousPosition.z);

    return xDiff < WORLD_VOXEL_SIZE || yDiff < WORLD_VOXEL_SIZE || zDiff < WORLD_VOXEL_SIZE;
}

float3 clampHistory(float3 history, float3 minColor, float3 maxColor)
{
    // Add controllable headroom to the bounds
    float3 coverage = (maxColor - minColor) * TEMPORAL_CLAMP_SCALE;
    minColor -= coverage;
    maxColor += coverage;

    return clamp(history, minColor, maxColor);
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
    RWTexture2D<float4> indirect = RWTexture2D<float4>::get(p.gbuffer.indirect);
    RWTexture2D<float4> indirectLast = RWTexture2D<float4>::get(p.gbuffer.indirectLast);
    RWTexture2D<float2> motion = RWTexture2D<float2>::get(p.gbuffer.motion);
    RWTexture2D<float> depth = RWTexture2D<float>::get(p.gbuffer.depth);
    RWTexture2D<int2> voxelIDs = RWTexture2D<int2>::get(p.gbuffer.voxelIDs);
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

    bool isVoxelFarLOD = IsObjectSmallerThanPixel(
        WORLD_VOXEL_SIZE * 0.25,
        trace.distance,
        p.state_ptr.camera.fov * 0.0174532925f,
        p.frame_dim.x);

    int voxelIndex = isVoxelFarLOD ? 0 : (trace.voxelIndex + 2);
    voxelIndex += (int(trace.normal.x * 127) << 0) + (int(trace.normal.y * 127) << 8) + (int(trace.normal.z * 127) << 16);

    // if (dot(trace.normal, rayDir) > (1 / trace.distance * 0.4) - 0.2)
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

    float3 bounceRayDir;
    bool shadow = false;
    TraceResult bounceShadow = traceVoxelRay(trace.hitPos + trace.normal * EPSILON, normalize(p.state_ptr.sunDir));
    shadow = bounceShadow.hit;

    if (all(pixel % 2 == 0))
    {
        float2 halfPixel = pixel / 2;
        float2 dimensions = p.frame_dim / 2;
        float4 blueNoiseSample = blueNoise[halfPixel % 128];

        TraceResult bounceResult = computeBounce(trace.hitPos, trace.normal, blueNoiseSample.xyz, p.state_ptr.frame, bounceRayDir);
        float3 rayBounceColor = calculateBounceColor(bounceResult, bounceRayDir);

        if (bounceResult.hit)
        {
            TraceResult bounceShadow = traceVoxelRay(bounceResult.hitPos + bounceResult.normal * EPSILON, normalize(p.state_ptr.sunDir));

            if (!bounceShadow.hit)
            {
                rayBounceColor += getSunColor(initAtmosphere(bounceShadow.normal, p.state_ptr.sunDir)) * 3;
            }
        }

        // Calculate previous position with subpixel precision
        float2 prevPos = prevPos * 0.5;
        // Get previous brick ID from the position
        int2 prevPixel = int2(prevPos);
        int2 prevVoxelData = voxelIDs[prevPixel * 2]; // Multiply by 2 since we're in half-res
        int prevBrickId = prevVoxelData.x;
        // Validate history
        bool valid = validateReprojection(position[pixel].xyz, position[prevPixel * 2].xyz);

        prevPos = clamp(prevPos, float2(0), dimensions - 1);

        // Sample lastFrame with bilinear interpolation
        float4 last = sampleHistoryBilinear(indirectLast, prevPos, dimensions);
        last = clamp(last, float4(0), float4(100));

        float blendFactor = (1 / (last.w + 1)) * 2;
        blendFactor = clamp(blendFactor, TEMPORAL_MIN_ALPHA, 1.0);

        // Final blend
        float3 blended = lerp(last.rgb, rayBounceColor, valid ? blendFactor : 1.0);
        indirect[halfPixel] = float4(blended, last.w + 1);
    }

    float3 albedoColor = getMaterial(trace, false);

    // write to the gbuffer
    albedo[pixel] = albedoColor;
    normal[pixel] = int3(normalize(trace.normal));
    position[pixel] = trace.hitPos;
    depth[pixel] = trace.distance;
    motion[pixel] = trace.hit ? motionPixels : float2(0, 0);
    shadowTex[pixel] = shadow ? 1.0 : 0.0;

    if (trace.hit)
    {
        voxelIDs[pixel] = int2(globalBrickIndex, voxelIndex);
    }
    else
    {
        voxelIDs[pixel] = int2(EMPTY_KEY, EMPTY_KEY);
    }
}
