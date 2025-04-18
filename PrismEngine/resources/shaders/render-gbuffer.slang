


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

// Normal Distribution Function (Trowbridge-Reitz GGX)
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH * NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = PI * denom * denom;

    return num / max(denom, EPSILON); // Avoid division by zero
}

// Geometry Function (Schlick-GGX)
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0f);
    float k = (r * r) / 8.0f; // Direct lighting

    float num   = NdotV;
    float denom = NdotV * (1.0f - k) + k;

    return num / max(denom, EPSILON); // Avoid division by zero
}

// Geometry Function (Smith's method) - Combines Schlick-GGX for light and view directions
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0f);
    float NdotL = max(dot(N, L), 0.0f);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Fresnel Function (Schlick approximation)
float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
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
        Material mat = getMaterial(hit.materialId);
        float3 albedoColor = mat.albedo;
        float metallic = mat.metallic;
        float roughness = mat.roughness;
        // Ensure roughness is not zero to avoid division by zero in PBR functions
        roughness = max(roughness, 0.01f); 

        // View and light directions
        float3 lightDir = normalize(p.stateBuffer.sunDir);
        float3 viewDir = -ray.direction;

        // Initialize atmosphere
        var atmosphere = initAtmosphere(lightDir, p.stateBuffer.sunDir);
        float3 sunColor = saturate(getSunColor(atmosphere));

        // Shadow calculation (Per-Voxel Face)
        // Calculate the center of the voxel containing the hit point
        float3 voxelBaseCorner = floor(worldPos / WORLD_VOXEL_SIZE) * WORLD_VOXEL_SIZE;
        float3 voxelCenterPos = voxelBaseCorner + float3(0.5f * WORLD_VOXEL_SIZE);

        // Calculate the center of the specific face hit by the ray
        float3 faceCenter = voxelCenterPos;
        float3 absNormal = abs(worldNormal);

        // Determine the position of the face boundary along the normal's primary axis
        float faceBoundaryCoord;
        if (absNormal.x > absNormal.y && absNormal.x > absNormal.z) { // X-face
            faceBoundaryCoord = round(worldPos.x / WORLD_VOXEL_SIZE) * WORLD_VOXEL_SIZE;
            faceCenter.x = faceBoundaryCoord;
        } else if (absNormal.y > absNormal.z) { // Y-face
            faceBoundaryCoord = round(worldPos.y / WORLD_VOXEL_SIZE) * WORLD_VOXEL_SIZE;
            faceCenter.y = faceBoundaryCoord;
        } else { // Z-face
            faceBoundaryCoord = round(worldPos.z / WORLD_VOXEL_SIZE) * WORLD_VOXEL_SIZE;
            faceCenter.z = faceBoundaryCoord;
        }

        Ray shadowRay;
        // Originate shadow ray from the calculated face center, offset slightly along the normal
        shadowRay.origin = faceCenter + worldNormal * 0.0001f;
        shadowRay.direction = lightDir;
        var shadowHit = traceScene(shadowRay, 0, false);

        // Add subtle normal variation using blue noise for more natural lighting
        uint uniqueIndex = hash(hit.voxelIndex ^ (hit.brickIndex << 16));
        uint blueNoiseIndex = uniqueIndex % 128;
        float3 perturbedNormal = normalize(worldNormal + 
            (blueNoiseStatic[blueNoiseIndex].xyw * 2.0 - 1.0) * NORMAL_NOISE_AMOUNT);
            
        // Calculate lighting
        // Calculate base ambient light (always present)
        float3 ambientLight = albedoColor * AMBIENT_LIGHT_LEVEL;
        float3 directLight = float3(0, 0, 0); // Initialize direct contribution to zero
        
        // Calculate direct sun light only if not shadowed
        if (!shadowHit.hit)
        {
            // Direct light - PBR calculation
            float3 N = perturbedNormal; // Use perturbed normal for lighting
            float3 V = viewDir;
            float3 L = lightDir;
            float3 H = normalize(V + L); // Halfway vector

            // Calculate PBR terms
            float NdotL = max(dot(N, L), 0.0f); // Calculate NdotL once here
            float NdotV = max(dot(N, V), 0.0f);
            float NdotH = max(dot(N, H), 0.0f);
            float HdotV = max(dot(H, V), 0.0f);

            // Reflectance at normal incidence (F0)
            // For non-metals (metallic = 0), F0 is typically low (e.g., 0.04 for dielectrics)
            // For metals (metallic = 1), F0 is the albedo color
            float3 F0 = lerp(float3(0.04f), albedoColor, metallic);

            // Cook-Torrance BRDF components
            float NDF = DistributionGGX(N, H, roughness);       
            float G   = GeometrySmith(N, V, L, roughness);      
            float3 F  = FresnelSchlick(HdotV, F0); // Use HdotV for Fresnel

            // Specular BRDF term
            float3 numerator    = NDF * G * F;
            float denominator = 4.0f * NdotV * NdotL + EPSILON; // Add epsilon to prevent division by zero
            float3 specular     = numerator / denominator;

            // Diffuse BRDF term (Lambertian, energy conserving with Fresnel)
            float3 kS = F; // Specular reflection fraction
            float3 kD = float3(1.0f) - kS; // Diffuse reflection fraction
            kD *= (1.0f - metallic); // Metals have no diffuse reflection

            float3 diffuse = kD * albedoColor / PI;

            // Final radiance calculation (Lo) = (kD * albedo / PI + specular) * NdotL * lightColor
            // Note: lightColor is sunColor here
            float3 Lo = (diffuse + specular) * NdotL * sunColor;

            // Assign PBR result as the direct light contribution
            directLight = Lo; 
        }

        // Final color assignment: Combine ambient and direct light
        float3 finalColor = ambientLight + directLight; 

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
        
        // Validate reprojection using depth comparison
        bool validReprojection = false;
        if (all(prevPixel >= 0 && prevPixel < p.screenSize))
        {
            float currentDepth = depth[pixel];
            float previousDepth = depth[uint2(prevPixel)]; // Sample depth at reprojected location
            
            // Use a relative depth threshold to account for varying distances
            // Allow a small difference (e.g., 2% of current depth, plus a small fixed tolerance)
            float depthThreshold = max(currentDepth * 0.02f, 0.1f); 
            
            if (abs(currentDepth - previousDepth) < depthThreshold)
            {
                validReprojection = true;
            }
        }

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
