


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
static const float TAA_BLEND_FACTOR = 0.2f;

// GTAO configuration
static const int GTAO_SLICES = 5;      // Number of slices (directions) around normal
static const int GTAO_STEPS = 5;       // Steps per slice along each direction
static const float GTAO_RADIUS = BRICK_SIZE * WORLD_VOXEL_SIZE * 4; // World space radius
static const float GTAO_FALLOFF_RANGE = 0.5f; // Falloff range as fraction of radius
static const float GTAO_INTENSITY = 0.9f;  // AO strength
static const float GTAO_POWER = 1.0f;  // Power for final value adjustment

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
    return all(diff < WORLD_VOXEL_SIZE * 30);
}

float2 calculateMotionClip(float3 worldPos, float4x4 viewProj, float4x4 lastViewProj)
{
    float4 currentClip = mul(viewProj, float4(worldPos, 1.0));
    float4 previousClip = mul(lastViewProj, float4(worldPos, 1.0));

    float2 currentPos = currentClip.xy / currentClip.w;
    float2 previousPos = previousClip.xy / previousClip.w;
    
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
    // ColorBounds bounds = findNeighborhoodBounds(current, pixel, uint2(dimensions));
    
    // Clamp history color to reduce ghosting
    previousColor = clamp(previousColor, 0, 1);
    
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

// Fast approximation of acos
float fastAcos(float x)
{
    // Approximation from https://seblagarde.wordpress.com/2014/12/01/inverse-trigonometric-functions-gpu-optimization-for-amd-gcn-architecture/
    float x_abs = abs(x);
    float res = -0.156583f * x_abs + PI * 0.5f;
    res *= sqrt(1.0f - x_abs);
    return (x >= 0) ? res : PI - res;
}

float calculateGTAO(uint2 pixel, float3 worldPos, float3 worldNormal, float depth,
                   RWTexture2D<float3> positionBuffer, RWTexture2D<float> depthBuffer,
                   float4 noiseVec, uint2 screenSize)
{
    // Early out for invalid depth
    if (depth <= 0.0f) return 1.0f;
    
    // All calculations will be done in WORLD SPACE for consistency
    // since our position buffer stores world space positions
    float3 viewDirection = normalize(p.stateBuffer.camera.position - worldPos); // From surface to camera in WORLD space
    
    // Calculate screen space radius based on world space radius
    float4 clipPos = mul(p.stateBuffer.camera.viewProj, float4(worldPos, 1.0f));
    float2 screenPos = (clipPos.xy / clipPos.w) * 0.5f + 0.5f;
    screenPos *= screenSize;
    
    // Calculate adaptive screen radius based on depth
    // Project world radius to screen space
    float4 radiusClipPos = mul(p.stateBuffer.camera.viewProj, float4(worldPos + float3(GTAO_RADIUS, 0, 0), 1.0f));
    float2 radiusScreenPos = (radiusClipPos.xy / radiusClipPos.w) * 0.5f + 0.5f;
    radiusScreenPos *= screenSize;
    float screenRadius = length(radiusScreenPos - screenPos);
    // Ensure minimum radius for close objects
    screenRadius = max(screenRadius, 30.0f);
    screenRadius = min(screenRadius, 200.0f); // Cap maximum to avoid too many samples
    
    float occlusion = 0.0f;
    
    // Main GTAO loop - for each slice direction
    for (int slice = 0; slice < GTAO_SLICES; slice++)
    {
        // Random angle for this slice
        float sliceAngle = (float(slice) + noiseVec.x) / float(GTAO_SLICES) * 2.0f * PI;
        float2 sliceDir = float2(cos(sliceAngle), sin(sliceAngle)); // Screen space direction
        
        // Track maximum horizon angles for this slice
        float maxCos = -1.0f; // Start with horizon below
        
        // Sample along the slice in screen space
        for (int step = 1; step <= GTAO_STEPS; step++)
        {
            // Use square root distribution for better near/far sample balance
            float stepRatio = float(step) / float(GTAO_STEPS);
            stepRatio = pow(stepRatio, 1.5f); // Power distribution for more samples near center
            
            // Add temporal noise to reduce banding
            float stepNoise = frac(noiseVec.y + float(slice * GTAO_STEPS + step) * 0.6180339887f);
            stepRatio = (float(step - 1) + stepNoise) / float(GTAO_STEPS);
            stepRatio = pow(stepRatio, 1.5f);
            
            // Screen space offset for this sample
            float2 sampleOffset = sliceDir * stepRatio * screenRadius;
            float2 samplePixel = float2(pixel) + sampleOffset;
            
            // Check bounds
            if (any(samplePixel < 0) || any(samplePixel >= screenSize))
                continue;
            
            // Sample the world position at this screen location
            uint2 sampleCoord = uint2(samplePixel);
            float3 sampleWorldPos = positionBuffer[sampleCoord];
            
            // Calculate horizon vector in WORLD SPACE
            float3 horizonVec = sampleWorldPos - worldPos;
            float horizonDist = length(horizonVec);
            
            // Skip if too close (self-occlusion) or invalid
            if (horizonDist < 0.01f)
                continue;
                
            horizonVec /= horizonDist; // Normalize
            
            // Calculate horizon angle using world space vectors
            float horizonCos = dot(worldNormal, horizonVec);
            
            // Smoother distance-based falloff
            float normalizedDist = horizonDist / GTAO_RADIUS;
            float weight = 1.0f;
            
            if (normalizedDist < 1.0f)
            {
                // Within radius - smooth falloff
                weight = 1.0f - pow(normalizedDist, 2.0f);
            }
            else if (normalizedDist < 1.0f + GTAO_FALLOFF_RANGE)
            {
                // In falloff range - smooth fade out
                float falloff = (normalizedDist - 1.0f) / GTAO_FALLOFF_RANGE;
                weight = 1.0f - falloff;
                weight = smoothstep(0.0f, 1.0f, weight);
            }
            else
            {
                // Beyond falloff - no contribution
                continue;
            }
            
            // Track maximum horizon angle (weighted)
            float weightedCos = lerp(-1.0f, horizonCos, weight);
            maxCos = max(maxCos, weightedCos);
        }
        
        // Calculate occlusion for this slice
        // Higher horizon angle = more occlusion
        float sliceOcclusion = saturate((maxCos + 1.0f) * 0.5f);
        occlusion += sliceOcclusion;
    }
    
    // Average occlusion over all slices
    occlusion /= float(GTAO_SLICES);
    
    // Apply power for contrast
    occlusion = pow(saturate(occlusion), GTAO_POWER);
    
    // Apply intensity and invert
    // Return: 1 = no occlusion, 0 = full occlusion
    float ao = 1.0f - occlusion * GTAO_INTENSITY;
    return saturate(ao);
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
    RWTexture2D<float3> indirectLightTex = RWTexture2D<float3>.get(p.gbuffer.indirectLight);
    RWTexture2D<float3> directLightTex = RWTexture2D<float3>.get(p.gbuffer.directLight);
    RWTexture2D<float4> blueNoise = RWTexture2D<float4>.get(p.blueNoise);
    RWTexture2D<float4> blueNoiseStatic = RWTexture2D<float4>.get(p.blueNoiseStatic);
    RWTexture2D<float> ambientOcclusionTex = RWTexture2D<float>.get(p.gbuffer.ao);
    RWTexture2D<float> ambientOcclusionDenoisedTex = RWTexture2D<float>.get(p.gbuffer.aoDenoised);

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
        float2 motionClip = calculateMotionClip(
            worldPos, 
            p.stateBuffer.camera.viewProj, 
            p.stateBuffer.lastCamera.viewProj
        );
        float2 motionPixels = motionClip * (p.screenSize * 0.5);
        if (length(motionPixels) < 0.0001 || isnan(motionPixels.x) || isnan(motionPixels.y))
        {
            motionPixels = float2(0, 0);
        }

        motion[pixel] = motionPixels;
        depth[pixel] = hit.distance;

        // Get material properties
        Material mat = getMaterial(hit.materialId);
        float3 albedoColor = mat.albedo;
        float metallic = mat.metallic;
        float roughness = mat.roughness;
        // Ensure roughness is not zero to avoid division by zero in PBR functions
        roughness = max(roughness, 0.01f); 

        albedo[pixel] = albedoColor;

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
        float3 perturbedNormal = normalize(worldNormal + (blueNoiseStatic[blueNoiseIndex].xyw * 2.0 - 1.0) * NORMAL_NOISE_AMOUNT);
        
        if (all((pixel % 2) == 0))
        {
            float ao = 0.0f;
            // Use GTAO for better quality and stability
            ao = calculateGTAO(pixel, worldPos, worldNormal, hit.distance, position, depth, noiseVec, p.screenSize);
            
            // Calculate the base position for the 2x2 block
            uint2 blockBase = (pixel / 2);
            ambientOcclusionTex[blockBase] = ao;
        }
        
        // Calculate lighting
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

        indirectLightTex[pixel] = AMBIENT_LIGHT_LEVEL;
        directLightTex[pixel] = directLight;
    }

    //--------------------------------------------------------------------------
    // Pass 1: Combine gbuffer
    //--------------------------------------------------------------------------
    else if (p.passNum == 1)
    {
        // Read GBuffer data
        float3 albedoColor = albedo[pixel];
        float3 indirectLightColor = indirectLightTex[pixel];
        float3 directLightColor = directLightTex[pixel];
        float ao = ambientOcclusionDenoisedTex[pixel / 2];
        
        // Apply AO to indirect lighting only (not direct sunlight)
        float3 finalColor = albedoColor * (indirectLightColor * ao + directLightColor);
        albedo[pixel] = albedoColor;
        currentFrame[pixel] = finalColor;
    }
    
    //--------------------------------------------------------------------------
    // Pass 2: Apply temporal anti-aliasing
    //--------------------------------------------------------------------------
    else if (p.passNum == 2)
    {
        float3 worldPos = position[pixel];
        float2 motionVector = motion[pixel];
        float3 finalColor = currentFrame[pixel];

        float2 prevPixel = float2(pixel) + motionVector;
        
        // Validate reprojection
        bool validReprojection = all(prevPixel >= 0 && prevPixel < p.screenSize) && validateReprojection(worldPos, position[uint2(prevPixel)]);

        if (validReprojection)
        {
            finalColor = applyTAA(currentFrame, history, pixel, prevPixel, p.screenSize, finalColor);
        }


        // Write final color
        albedo[pixel] = finalColor;
        currentFrame[pixel] = finalColor;
    }
}