


#include "tracing/trace.slang"
#include "atmosphere.slang"

//------------------------------------------------------------------------------
// Constants and Configuration
//------------------------------------------------------------------------------

// Vectors and thresholds
static const float3 UP_VECTOR = float3(0, 1, 0);
static const float3 FORWARD_VECTOR = float3(0, 0, 1);
static const float PARALLEL_THRESHOLD = 0.9999f;

// TAA constants
static const float TAA_BLEND_FACTOR = 0.1f;
static const float TAA_MOTION_REJECTION_THRESHOLD = 0.5f;

// AO configuration 
static const int AO_SAMPLE_COUNT = 1;  // Increased from 1 for better quality
static const float AO_RADIUS = 4.0f;
static const float AO_INTENSITY = 1.0f;
static const float AO_BIAS = 0.001f;
static const float AO_DISTANCE_SCALING = 0.8f;

// Lighting parameters
static const float SPECULAR_POWER = 32.0f;
static const float SPECULAR_INTENSITY = 0.5f;
static const float AMBIENT_LIGHT_LEVEL = 0.1f;
static const float NORMAL_NOISE_AMOUNT = 0.03f;

//------------------------------------------------------------------------------
// Motion and Reprojection
//------------------------------------------------------------------------------

bool validateReprojection(float3 currentPosition, float3 previousPosition)
{
    float3 diff = abs(currentPosition - previousPosition);
    return all(diff < WORLD_VOXEL_SIZE);
}

float2 calculateMotionVector(float3 worldPos, float4x4 viewProj, float4x4 lastViewProj, 
                            float2 currentJitter, float2 previousJitter)
{
    float4 currentClip = mul(viewProj, float4(worldPos, 1.0));
    float4 previousClip = mul(lastViewProj, float4(worldPos, 1.0));

    float2 currentPos = currentClip.xy / currentClip.w;
    float2 previousPos = previousClip.xy / previousClip.w;

    // Add jitter offset difference
    float2 jitterOffset = (previousJitter - currentJitter) / float2(p.screenSize) * 2.0f;
    
    return (previousPos - currentPos);
}

//------------------------------------------------------------------------------
// TAA Implementation
//------------------------------------------------------------------------------

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
    int2 c10 = clamp(baseCoord + int2(1, 0), int2(0, 0), int2(dimensions) - 1);
    int2 c01 = clamp(baseCoord + int2(0, 1), int2(0, 0), int2(dimensions) - 1);
    int2 c11 = clamp(baseCoord + int2(1, 1), int2(0, 0), int2(dimensions) - 1);

    // Sample nearest pixels
    float3 s00 = history[c00];
    float3 s10 = history[c10];
    float3 s01 = history[c01];
    float3 s11 = history[c11];

    // Bilinear blend
    float3 s0 = lerp(s00, s10, weights.x);
    float3 s1 = lerp(s01, s11, weights.x);
    return lerp(s0, s1, weights.y);
}

struct ColorBounds {
    float3 min;
    float3 max;
};

ColorBounds findNeighborhoodBounds(RWTexture2D<float3> current, uint2 pixel, uint2 dimensions)
{
    ColorBounds bounds;
    bounds.min = float3(1.0, 1.0, 1.0);
    bounds.max = float3(0.0, 0.0, 0.0);
    
    // Use a compact 3x3 neighborhood for better performance
    const int radius = 1;
    
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
    
    // Slightly expand bounds to reduce flickering
    float3 boundDelta = (bounds.max - bounds.min) * 0.1f;
    bounds.min -= boundDelta;
    bounds.max += boundDelta;
    
    return bounds;
}

float3 applyTAA(RWTexture2D<float3> current, RWTexture2D<float3> history, 
                uint2 pixel, float2 previousPixel, float2 dimensions, float3 currentColor)
{
    // Sample history color using bilinear filtering
    float3 previousColor = sampleHistoryBilinear(history, previousPixel, dimensions);
    
    // Find neighborhood bounds for color clamping
    ColorBounds bounds = findNeighborhoodBounds(current, pixel, uint2(dimensions));
    
    // Clamp history color to reduce ghosting
    previousColor = clamp(previousColor, bounds.min, bounds.max);
    
    // Blend between current and history colors
    return lerp(previousColor, currentColor, TAA_BLEND_FACTOR);
}

//------------------------------------------------------------------------------
// Lighting Helpers
//------------------------------------------------------------------------------

float3x3 createRotationFromNormal(float3 normal)
{
    normal = normalize(normal);
    
    float upDot = dot(normal, UP_VECTOR);
    float3 right;
    
    if (abs(upDot) > PARALLEL_THRESHOLD)
    {
        right = normalize(cross(FORWARD_VECTOR, normal));
    }
    else
    {
        right = normalize(cross(UP_VECTOR, normal));
    }
    
    float3 forward = normalize(cross(normal, right));
    
    return float3x3(
        right.x, normal.x, forward.x,
        right.y, normal.y, forward.y,
        right.z, normal.z, forward.z);
}

float calculateAmbientOcclusion(float3 position, float distance, float3 normal, float4 noiseVec)
{
    float occlusion = 0.0f;
    float adaptiveRadius = AO_RADIUS * min(1.0f, AO_RADIUS / distance);
    
    // Adaptively reduce sample count based on distance for better performance
    int sampleCount = AO_SAMPLE_COUNT;
    if (distance > 20) sampleCount = max(2, sampleCount / 2);
    if (distance > 40) sampleCount = max(1, sampleCount / 2);
    
    float3x3 rotation = createRotationFromNormal(normal);
    
    for (int i = 0; i < sampleCount; i++)
    {
        // Generate sample direction using blue noise for better distribution
        float3 sampleDir = noiseVec.xyz;
        sampleDir = float3(sampleDir.x * 2.0f - 1.0f, sampleDir.y - 0.2f, sampleDir.z * 2.0f - 1.0f);
        sampleDir = normalize(sampleDir);
        
        // Rotate sample direction to create better distribution
        float angle = noiseVec.w * 2 * PI * float(i) / float(sampleCount);
        float s = sin(angle);
        float c = cos(angle);
        sampleDir = float3(sampleDir.x * c - sampleDir.z * s, sampleDir.y, sampleDir.x * s + sampleDir.z * c);
        
        // Transform sample to align with normal
        sampleDir = mul(rotation, sampleDir);
        
        // Create ray for occlusion testing
        Ray aoRay;
        aoRay.origin = position + normal * AO_BIAS;
        aoRay.direction = sampleDir;
        
        // Use adaptive ray length based on distance
        aoRay.maxIterations = 8;
        if (distance > 10) aoRay.maxIterations = 6;
        if (distance > 40) aoRay.maxIterations = 4;
        
        // Trace the ray
        var aoHit = traceScene(aoRay, 0, false, false);
        
        // Accumulate occlusion with distance-based falloff
        if (aoHit.hit && aoHit.distance < adaptiveRadius)
        {
            float weight = 1.0f - pow(aoHit.distance / adaptiveRadius, AO_DISTANCE_SCALING);
            occlusion += weight;
        }
    }
    
    // Normalize and apply intensity
    occlusion = occlusion / float(sampleCount);
    occlusion = pow(occlusion, 1.5f); // Adjusts occlusion curve for better appearance
    occlusion *= AO_INTENSITY;
    
    // Return visibility factor
    return saturate(1.0f - occlusion);
}

uint hash(uint x) {
    x ^= x >> 16;
    x *= 0x85ebca6b;
    x ^= x >> 13; 
    x *= 0xc2b2ae35;
    x ^= x >> 16;
    return x;
}

float3 calculateSpecular(float3 viewDirection, float3 normal, float3 lightDirection, float specularPower, float specularIntensity)
{
    float3 halfVector = normalize(viewDirection + lightDirection);
    float NdotH = max(dot(normal, halfVector), 0.0f);
    
    // Use smoother specular falloff for more pleasing highlights
    float specularFactor = pow(NdotH, specularPower);
    
    return float3(specularIntensity * specularFactor);
}

//------------------------------------------------------------------------------
// Main Shader
//------------------------------------------------------------------------------

[numthreads(32, 32, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    uint2 pixel = pixel_i.xy;
    if (any(pixel >= p.screenSize))
        return;

    // Get G-buffer textures
    RWTexture2D<float3> albedo = RWTexture2D<float3>.get(p.gbuffer.color);
    RWTexture2D<float3> normalTex = RWTexture2D<float3>.get(p.gbuffer.normal);
    RWTexture2D<float3> position = RWTexture2D<float3>.get(p.gbuffer.position);
    RWTexture2D<float2> motion = RWTexture2D<float2>.get(p.gbuffer.motion);
    RWTexture2D<float> depth = RWTexture2D<float>.get(p.gbuffer.depth);
    RWTexture2D<float3> history = RWTexture2D<float3>.get(p.gbuffer.history);
    RWTexture2D<float3> currentFrame = RWTexture2D<float3>.get(p.gbuffer.currentFrame);
    RWTexture2D<float4> blueNoise = RWTexture2D<float4>.get(p.blueNoise);
    RWTexture2D<float4> blueNoiseStatic = RWTexture2D<float4>.get(p.blueNoiseStatic);

    float4 noiseVec = blueNoise[pixel % 128];

    //--------------------------------------------------------------------------
    // Pass 0: Generate GBuffer and initial color
    //--------------------------------------------------------------------------
    if (p.passNum == 0)
    {
        // Generate primary ray
        var ray = Ray.FromPixelJitter(pixel, p.screenSize, p.stateBuffer.camera);
        var hit = traceScene(ray, 0, false);

        // Early out for sky/background
        if (!hit.hit)
        {
            normalTex[pixel] = 0;
            position[pixel] = 0;
            motion[pixel] = 0;
            depth[pixel] = -10;
            return;
        }
        
        // Extract hit information
        float3 worldNormal = hit.normal;
        float3 worldPos = ray.origin + ray.direction * hit.distance;
        
        // Write GBuffer data
        normalTex[pixel] = worldNormal;
        position[pixel] = worldPos;
        
        // Calculate motion vectors for TAA
        float2 currentJitter = Ray.getJitterOffset(p.stateBuffer.frame);
        float2 previousJitter = Ray.getJitterOffset(p.stateBuffer.frame - 1);
        float2 motionVector = calculateMotionVector(
            worldPos, 
            p.stateBuffer.camera.viewProj, 
            p.stateBuffer.lastCamera.viewProj, 
            currentJitter, 
            previousJitter
        );

        motion[pixel] = motionVector;
        depth[pixel] = hit.distance;

        // Get material properties
        float3 baseColor = getMaterial(hit.materialId).albedo;
        float specularStrength = 1.0f; // Can be adjusted from material properties

        // View and light directions
        float3 lightDir = normalize(p.stateBuffer.sunDir);
        float3 viewDir = -ray.direction;

        // Initialize atmosphere
        var atmosphere = initAtmosphere(lightDir, p.stateBuffer.sunDir);
        float3 sunColor = saturate(getSunColor(atmosphere));
        
        // Shadow calculation
        Ray shadowRay;
        shadowRay.origin = worldPos + worldNormal * 0.0001f; // Offset to avoid self-intersection
        shadowRay.direction = lightDir;
        var shadowHit = traceScene(shadowRay, 0, false);
        
        // Add subtle normal variation using blue noise for more natural lighting
        uint uniqueIndex = hash(hit.voxelIndex ^ (hit.brickIndex << 16));
        uint blueNoiseIndex = uniqueIndex % 128;
        float3 perturbedNormal = normalize(worldNormal + 
            (blueNoiseStatic[blueNoiseIndex].xyw * 2.0 - 1.0) * NORMAL_NOISE_AMOUNT);
            
        // Calculate lighting
        float3 diffuseColor;
        float3 specularColor = float3(0, 0, 0);
        
        // Direct lighting calculation (diffuse + specular)
        if (shadowHit.hit)
        {
            // In shadow - only ambient light
            diffuseColor = baseColor * AMBIENT_LIGHT_LEVEL;
        }
        else
        {
            // Direct light - diffuse term
            float NdotL = max(dot(perturbedNormal, lightDir), 0.0);
            diffuseColor = baseColor * NdotL * sunColor;
            
            // Add specular highlight for non-shadowed areas
            specularColor = calculateSpecular(viewDir, perturbedNormal, lightDir, 
                SPECULAR_POWER, SPECULAR_INTENSITY * specularStrength) * sunColor;
        }

        // Calculate ambient occlusion
        float ao = calculateAmbientOcclusion(worldPos, hit.distance, worldNormal, noiseVec);
        
        // Apply AO and atmosphere for better visual quality
        if (!shadowHit.hit)
        {
            // Blend atmosphere influence based on AO
            float aoFactor = 1.5 - ao;
            ao = lerp(1.0f, aoFactor, 0.5); // Keep ao as a scalar value
        }

        // Combine all lighting components
        float3 finalColor;
        if (!shadowHit.hit) {
            // Apply AO to diffuse and add atmosphere influence
            finalColor = (diffuseColor * ao * sunColor) + specularColor;
        } else {
            // In shadow - just apply AO to ambient light
            finalColor = diffuseColor * ao;
        }
        
        // Store result
        currentFrame[pixel] = finalColor;
    }
    
    //--------------------------------------------------------------------------
    // Pass 1: Apply temporal anti-aliasing
    //--------------------------------------------------------------------------
    else if (p.passNum == 1)
    {
        float3 worldPos = position[pixel];
        float2 motionVector = motion[pixel];
        float3 currentColor = currentFrame[pixel];

        // Calculate previous pixel position
        float2 motionPixels = motionVector * (p.screenSize * 0.5);
        
        // Check for invalid motion vectors
        if (length(motionPixels) < 0.0001 || any(isnan(motionPixels)))
        {
            motionPixels = float2(0, 0);
        }

        float2 prevPixel = float2(pixel) + motionPixels;
        
        // Validate reprojection
        bool validReprojection = all(prevPixel >= 0 && prevPixel < p.screenSize) && 
                                validateReprojection(worldPos, position[uint2(prevPixel)]);

        // Apply TAA only if reprojection is valid
        float3 finalColor = currentColor;
        if (validReprojection)
        {
            finalColor = applyTAA(currentFrame, history, pixel, prevPixel, p.screenSize, currentColor);
        }

        // Write final color
        albedo[pixel] = finalColor;
        currentFrame[pixel] = finalColor;
    }
}