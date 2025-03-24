#include "tracing/trace.slang"

static const float3 UP_VECTOR = float3(0, 1, 0);
static const float3 FORWARD_VECTOR = float3(0, 0, 1);
static const float PARALLEL_THRESHOLD = 0.9999f;
static const float TAA_BLEND_FACTOR = 1/16.0f;

bool validateReprojection(float3 currentPosition, float3 previousPosition)
{
    float xDiff = abs(currentPosition.x - previousPosition.x) * 10;
    float yDiff = abs(currentPosition.y - previousPosition.y) * 10;
    float zDiff = abs(currentPosition.z - previousPosition.z) * 10;

    return xDiff < WORLD_VOXEL_SIZE || yDiff < WORLD_VOXEL_SIZE || zDiff < WORLD_VOXEL_SIZE;
}

float2 calculateMotionClip(float3 worldPos, float4x4 viewProj, float4x4 lastViewProj, float2 currentJitter, float2 previousJitter)
{
    float4 currentClip = mul(viewProj, float4(worldPos, 1.0));
    float4 previousClip = mul(lastViewProj, float4(worldPos, 1.0));

    float2 currentPos = currentClip.xy / currentClip.w;
    float2 previousPos = previousClip.xy / previousClip.w;

    // Add jitter offset difference to motion vector
    float2 jitterOffset = (previousJitter - currentJitter) / float2(p.frame_dim) * 2.0f;
    
    return (previousPos - currentPos);
}

void getBilinearWeights(float2 pos, out int2 baseCoord, out float2 weights)
{
    baseCoord = int2(floor(pos));
    weights = frac(pos);
}

float3 sampleHistoryBilinear(RWTexture2D<float3> history, float2 pos, float2 dimensions)
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
    float3 s00 = history[c00];
    float3 s10 = history[c10];
    float3 s01 = history[c01];
    float3 s11 = history[c11];

    // Bilinear blend
    float3 s0 = lerp(s00, s10, weights.x);
    float3 s1 = lerp(s01, s11, weights.x);
    return lerp(s0, s1, weights.y);
}

// Find minimum and maximum color in neighborhood for clamping
struct ColorBounds {
    float3 min;
    float3 max;
};

ColorBounds findNeighborhoodBounds(RWTexture2D<float3> current, uint2 pixel, uint2 dimensions, int radius)
{
    ColorBounds bounds;
    bounds.min = float3(1.0, 1.0, 1.0);
    bounds.max = float3(0.0, 0.0, 0.0);
    
    for (int y = -radius; y <= radius; y++)
    {
        for (int x = -radius; x <= radius; x++)
        {
            uint2 samplePos = clamp(int2(pixel) + int2(x, y), int2(0, 0), int2(dimensions) - 1);
            float3 sampleColor = current[samplePos];
            
            bounds.min = min(bounds.min, sampleColor);
            bounds.max = max(bounds.max, sampleColor);
        }
    }
    
    return bounds;
}

// Clamp history color to neighborhood bounds to reduce ghosting
float3 clipToNeighborhood(float3 color, ColorBounds bounds)
{
    return clamp(color, bounds.min, bounds.max);
}

float3 applyTAA(RWTexture2D<float3> current, RWTexture2D<float3> history, 
                uint2 pixel, float2 previousPixel, float2 dimensions, float3 currentColor)
{
    // Sample history color using bilinear filtering
    float3 previousColor = sampleHistoryBilinear(history, previousPixel, dimensions);
    
    // Find neighborhood bounds for color clamping (use 3x3 neighborhood)
    // ColorBounds bounds = findNeighborhoodBounds(current, pixel, dimensions, 1);
    
    // Clamp history color to neighborhood bounds to reduce ghosting
    // previousColor = clipToNeighborhood(previousColor, bounds);
    
    // Blend between current and history colors
    return lerp(previousColor, currentColor, TAA_BLEND_FACTOR);
}

// Configuration parameters for AO
static const int AO_SAMPLE_COUNT = 1;
static const float AO_RADIUS = 100.0f;
static const float AO_INTENSITY = 1.0f;
static const float AO_BIAS = 0.001;


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

float calculateAmbientOcclusion(float3 position, float3 normal, uint2 pixel)
{
    float occlusion = 0.0f;
    
    // Get random rotation vector from blue noise texture
    RWTexture2D<float4> blueNoise = RWTexture2D<float4>.get(p.blueNoise);
    float4 noiseVec = blueNoise[pixel % 64];
    
    // Sample points around the hemisphere
    for (int i = 0; i < AO_SAMPLE_COUNT; i++)
    {
        float3 sampleDir = noiseVec.xyz;
        sampleDir = float3(sampleDir.x * 2.0f - 1.0f, sampleDir.y - 0.2f, sampleDir.z * 2.0f - 1.0f);
        sampleDir = normalize(sampleDir);

        // rotate ray around y axis
        float angle = noiseVec.w * 2 * PI * float(i) / float(AO_SAMPLE_COUNT);
        float s = sin(angle);
        float c = cos(angle);
        sampleDir = float3(sampleDir.x * c - sampleDir.z * s, sampleDir.y, sampleDir.x * s + sampleDir.z * c);

        // Rotate sample direction to match normal
        float3x3 rotation = CreateRotationFromNormal3x3(normal);
        sampleDir = mul(rotation, sampleDir);
        
        // Create the ray for occlusion testing
        Ray aoRay;
        aoRay.origin = position + normal * AO_BIAS; // Avoid self-intersection
        aoRay.direction = sampleDir;
        aoRay.maxDist = 50 * AO_RADIUS;
        
        // Trace the ray with limited distance (AO_RADIUS)
        var aoHit = traceScene(aoRay, 0, false, false);
        
        // If we hit something within our AO radius, it contributes to occlusion
        if (aoHit.hit && aoHit.distance < AO_RADIUS)
        {
            // Weight by distance - farther hits occlude less
            float weight = 1.0f - (aoHit.distance / AO_RADIUS);
            occlusion += weight;
        }
    }
    
    // Normalize and apply intensity
    occlusion = occlusion / float(AO_SAMPLE_COUNT);
    occlusion = pow(occlusion, 1.5f); // Optional: Adjust occlusion curve
    occlusion *= AO_INTENSITY;
    
    // Return visibility (1.0 = fully visible, 0.0 = fully occluded)
    return 1.0f - occlusion;
}



[numthreads(32, 32, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    uint2 pixel = pixel_i.xy;
    if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
        return;

    RWTexture2D<float3> albedo = RWTexture2D<float3>.get(p.gbuffer.color);
    RWTexture2D<float3> normalTex = RWTexture2D<float3>.get(p.gbuffer.normal);
    RWTexture2D<float3> position = RWTexture2D<float3>.get(p.gbuffer.position);
    RWTexture2D<float2> motion = RWTexture2D<float2>.get(p.gbuffer.motion);
    RWTexture2D<float> depth = RWTexture2D<float>.get(p.gbuffer.depth);
    RWTexture2D<float3> history = RWTexture2D<float3>.get(p.gbuffer.history);
    RWTexture2D<float3> currentFrame = RWTexture2D<float3>.get(p.gbuffer.currentFrame);

    // First pass: Generate GBuffer data and initial color
    if (p.pass == 0)
    {
        var ray = Ray.FromPixelJitter(pixel, p.frame_dim, p.state_ptr.camera);
        var hit = traceScene(ray, 0, false);

        if (!hit.hit)
        {
            normalTex[pixel] = 0;
            position[pixel] = 0;
            motion[pixel] = 0;
            depth[pixel] = -10;
            return;
        }
        
        float3 worldNormal = hit.normal;
        float3 worldPos = ray.origin + ray.direction * hit.distance;
        
        // Write GBuffer data
        normalTex[pixel] = worldNormal;
        position[pixel] = worldPos;
        // In the first pass (p.pass == 0)
        float2 currentJitter = Ray.getJitterOffset(p.state_ptr.frame);
        float2 previousJitter = Ray.getJitterOffset(p.state_ptr.frame - 1);
        float2 motionClip = calculateMotionClip(worldPos, p.state_ptr.camera.viewProj, p.state_ptr.lastCamera.viewProj, currentJitter, previousJitter);

        motion[pixel] = motionClip;
        depth[pixel] = hit.distance;

        

        // Get base color from material
        float3 baseColor = getMaterial(hit.materialId).albedo;

        // shadow
        // first calc dot product between normal and light direction
        float3 lightDir = normalize(p.state_ptr.sunDir);
        float dot = dot(worldNormal, lightDir);
        // baseColor *= max(dot, 0.2);

        // cast shadow ray
        Ray shadowRay;
        shadowRay.origin = worldPos + worldNormal * 0.0001f;
        shadowRay.direction = lightDir;
        var shadowHit = traceScene(shadowRay, 0, false);
        
        if (shadowHit.hit)
        {
            baseColor *= 0.2f;
        }

        // Apply ambient occlusion
        float ao = calculateAmbientOcclusion(worldPos, worldNormal, pixel);
        baseColor *= ao + ((float)!shadowHit.hit) * 0.6f;
        currentFrame[pixel] = baseColor;
    }

    // Second pass: Apply temporal anti-aliasing
    else if (p.pass == 1)
    {
        float3 worldPos = position[pixel];
        float2 motionClip = motion[pixel];
        float3 baseColor = currentFrame[pixel];

        float2 motionPixels = motionClip * (p.frame_dim * 0.5);
        if (length(motionPixels) < 0.0001 || isnan(motionPixels.x) || isnan(motionPixels.y))
        {
            motionPixels = float2(0, 0);
        }

        float2 prevPixel = float2(pixel) + float2(motionPixels);
        bool valid = validateReprojection(worldPos, position[prevPixel]);

        float3 finalColor;
        if (valid)
        {
            finalColor = applyTAA(albedo, history, pixel, prevPixel, p.frame_dim, baseColor);
        } 
        else
        {
            finalColor = baseColor;
        }

        // Write final color to output
        albedo[pixel] = finalColor;
        currentFrame[pixel] = finalColor;
    }
}