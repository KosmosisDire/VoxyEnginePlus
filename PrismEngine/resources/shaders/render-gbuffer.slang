#include "common.slang"
#include "tracing/trace-contree.slang"

//------------------------------------------------------------------------------
// GTAO Configuration
//------------------------------------------------------------------------------
static const int GTAO_SLICES = 5;      // Number of slices (directions) around normal
static const int GTAO_STEPS = 2;       // Steps per slice along each direction
static const float GTAO_RADIUS = BRICK_SIZE * WORLD_VOXEL_SIZE * 4; // World space radius
static const float GTAO_FALLOFF_RANGE = 0.5f; // Falloff range as fraction of radius
static const float GTAO_INTENSITY = 0.9f;  // AO strength
static const float GTAO_POWER = 1.0f;  // Power for final value adjustment

//------------------------------------------------------------------------------
// GTAO Implementation
//------------------------------------------------------------------------------
float calculateGTAO(uint2 pixel, float3 worldPos, float3 worldNormal, float depth,
                   RWTexture2D<float3> positionBuffer, RWTexture2D<float> depthBuffer,
                   float4 noiseVec, uint2 screenSize)
{
    // Early out for invalid depth
    if (depth <= 0.0f) return 1.0f;

    // All calculations in WORLD SPACE
    float3 viewDirection = normalize(p.stateBuffer.camera.position - worldPos);

    // Calculate screen space radius based on world space radius
    float4 clipPos = mul(p.stateBuffer.camera.viewProj, float4(worldPos, 1.0f));
    float2 screenPos = (clipPos.xy / clipPos.w) * 0.5f + 0.5f;
    screenPos *= screenSize;

    // Project world radius to screen space
    float4 radiusClipPos = mul(p.stateBuffer.camera.viewProj, float4(worldPos + float3(GTAO_RADIUS, 0, 0), 1.0f));
    float2 radiusScreenPos = (radiusClipPos.xy / radiusClipPos.w) * 0.5f + 0.5f;
    radiusScreenPos *= screenSize;
    float screenRadius = length(radiusScreenPos - screenPos);
    screenRadius = max(screenRadius, 30.0f);  // Minimum radius
    screenRadius = min(screenRadius, 200.0f); // Maximum radius

    float occlusion = 0.0f;

    // Main GTAO loop - for each slice direction
    for (int slice = 0; slice < GTAO_SLICES; slice++)
    {
        // Random angle for this slice using blue noise
        float sliceAngle = (float(slice) + noiseVec.x) / float(GTAO_SLICES) * 2.0f * PI;
        float2 sliceDir = float2(cos(sliceAngle), sin(sliceAngle));

        // Track maximum horizon angles for this slice
        float maxCos = -1.0f;

        // Sample along the slice in screen space
        for (int step = 1; step <= GTAO_STEPS; step++)
        {
            // Power distribution for better sampling
            float stepRatio = float(step) / float(GTAO_STEPS);
            stepRatio = pow(stepRatio, 1.5f);

            // Add noise to reduce banding
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

            // Distance-based falloff
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

//------------------------------------------------------------------------------
// Main Shader - Pass 0: Calculate AO only
//------------------------------------------------------------------------------
[numthreads(8, 8, 1)]  // 64 threads per workgroup for shared memory optimization
void main(uint3 pixel_i: SV_DispatchThreadID, uint3 group_thread_id: SV_GroupThreadID)
{
    uint2 pixel = pixel_i.xy;
    if (any(pixel >= p.screenSize))
        return;

    RWTexture2D<float3> positionTex = RWTexture2D<float3>.get(p.gbuffer.position);
    RWTexture2D<float3> normalTex = RWTexture2D<float3>.get(p.gbuffer.normal);
    RWTexture2D<float> depthTex = RWTexture2D<float>.get(p.gbuffer.depth);
    RWTexture2D<float> aoTex = RWTexture2D<float>.get(p.gbuffer.ao);
    RWTexture2D<float4> blueNoise = RWTexture2D<float4>.get(p.blueNoise);
    RWTexture2D<float3> colorTex = RWTexture2D<float3>.get(p.gbuffer.color);

    // Pass 0: Trace rays and write GBuffer + lighting
    if (p.passNum == 0)
    {
        // Calculate thread index for shared memory optimization
        // 8x8 workgroup = 64 threads, flatten 2D index to 1D
        uint threadIndex = group_thread_id.x + group_thread_id.y * 8;

        // Generate primary ray
        var ray = Ray.FromPixel(pixel, p.screenSize, p.stateBuffer.camera);
        ray.maxIterations = 200;

        var hit = traceContree(ray, threadIndex);

        // Handle miss - sky
        if (!hit.hit)
        {
            colorTex[pixel] = float3(0.5f, 0.7f, 1.0f); // Sky blue
            normalTex[pixel] = 0;
            positionTex[pixel] = 0;
            depthTex[pixel] = -1;
            return;
        }

        // Extract hit information
        float3 worldNormal = hit.normal;
        float3 worldPos = hit.position;

        // Write GBuffer data
        normalTex[pixel] = worldNormal;
        positionTex[pixel] = worldPos;
        depthTex[pixel] = hit.distance;

        // Get material properties (default to white for now)
        float3 albedoColor = float3(0.8f, 0.8f, 0.8f);

        // If we have material data, use it
        if (hit.materialId > 0 && hit.materialId < 256) {
            Material mat = getMaterial(hit.materialId);
            albedoColor = mat.albedo;
        }

        // Light direction (sun from above and to the side)
        float3 lightDir = normalize(p.stateBuffer.sunDir);

        // Cast shadow ray (reuse same threadIndex)
        Ray shadowRay;
        shadowRay.origin = worldPos + worldNormal * 0.1f; // Offset to avoid self-intersection
        shadowRay.direction = lightDir;
        shadowRay.maxDist = 1000.0f;
        shadowRay.maxIterations = 100;

        // Use coarse mode for shadows - early exit after 20 iterations if we hit anything
        var shadowHit = traceContree(shadowRay, threadIndex, true);
        float shadow = shadowHit.hit ? 0.3f : 1.0f; // Soft shadow

        // Diffuse lighting (Lambert)
        float ndotl = max(dot(worldNormal, lightDir), 0.0f);
        float3 diffuse = albedoColor * ndotl * shadow;

        // Specular lighting (Blinn-Phong)
        float3 viewDir = normalize(p.stateBuffer.camera.position - worldPos);
        float3 halfDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(worldNormal, halfDir), 0.0f), 32.0f);
        float3 specular = float3(0.3f, 0.3f, 0.3f) * spec * shadow;

        // Ambient lighting
        float3 ambient = albedoColor * 0.15f;

        // Sky lighting (hemisphere lighting)
        float skyFactor = worldNormal.y * 0.5f + 0.5f; // Map [-1,1] to [0,1]
        float3 skyColor = float3(0.5f, 0.7f, 1.0f) * 0.2f;
        float3 skyLight = albedoColor * skyColor * skyFactor;

        // Combine all lighting
        float3 finalColor = diffuse + specular + ambient + skyLight;

        // Store lighting (AO will be applied in pass 2)
        colorTex[pixel] = finalColor;
    }
    // Pass 1: Calculate GTAO from complete GBuffer
    else if (p.passNum == 1)
    {
        // Only calculate AO for every 2x2 pixels (half-res)
        if (!all((pixel % 2) == 0))
            return;

        float depth = depthTex[pixel];

        // Skip sky
        if (depth < 0)
            return;

        float3 worldPos = positionTex[pixel];
        float3 worldNormal = normalTex[pixel];

        float4 noiseVec = blueNoise[pixel % 128];
        float ao = calculateGTAO(pixel, worldPos, worldNormal, depth, positionTex, depthTex, noiseVec, p.screenSize);
        aoTex[pixel / 2] = ao;
    }
    // Pass 2: Apply AO to all pixels
    else if (p.passNum == 2)
    {
        float depth = depthTex[pixel];

        // Skip sky
        if (depth < 0)
            return;

        // Read the lighting we stored
        float3 color = colorTex[pixel];

        // Bilinear sampling from half-res AO buffer
        // Convert pixel coord to half-res space
        float2 halfResCoord = float2(pixel) * 0.5f;
        uint2 coord00 = uint2(floor(halfResCoord));
        uint2 coord10 = coord00 + uint2(1, 0);
        uint2 coord01 = coord00 + uint2(0, 1);
        uint2 coord11 = coord00 + uint2(1, 1);

        // Get fractional part for interpolation
        float2 frac = halfResCoord - floor(halfResCoord);

        // Sample 4 nearest AO values
        float ao00 = aoTex[coord00];
        float ao10 = aoTex[coord10];
        float ao01 = aoTex[coord01];
        float ao11 = aoTex[coord11];

        // Bilinear interpolation
        float ao0 = lerp(ao00, ao10, frac.x);
        float ao1 = lerp(ao01, ao11, frac.x);
        float ao = lerp(ao0, ao1, frac.y);

        // Apply AO to lighting
        colorTex[pixel] = color * ao;
    }
}
