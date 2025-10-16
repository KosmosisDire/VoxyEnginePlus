#include "common.slang"
#include "tracing/trace-contree.slang"

//------------------------------------------------------------------------------
// GTAO Configuration (based on XeGTAO reference implementation)
//------------------------------------------------------------------------------
static const int GTAO_SLICES = 3;      // Number of slices (directions) around normal
static const int GTAO_STEPS = 3;       // Steps per slice along each direction
static const float GTAO_RADIUS = 3.5f; // VIEW SPACE radius in units
static const float GTAO_FALLOFF_RANGE = 0.6f; // Falloff range as fraction of radius (XeGTAO default)
static const float GTAO_SAMPLE_DISTRIBUTION_POWER = 2.0f; // Focus samples towards center
static const float GTAO_THIN_OCCLUDER_COMPENSATION = 0.0f; // Reduce over-darkening from thin occluders
static const float GTAO_FINAL_VALUE_POWER = 1.5f; // Power for final value adjustment
static const float GTAO_PIXEL_TOO_CLOSE_THRESHOLD = 1.9f; // Minimum distance threshold

// Fast approximation of acos for GTAO (from XeGTAO)
float FastACos(float inX)
{
    const float PI = 3.141593f;
    const float HALF_PI = 1.570796f;
    float x = abs(inX);
    float res = -0.156583f * x + HALF_PI;
    res *= sqrt(1.0f - x);
    return (inX >= 0) ? res : PI - res;
}

// Convert world position to view space
float3 WorldToViewSpace(float3 worldPos)
{
    float4 viewPos = mul(p.stateBuffer.camera.view, float4(worldPos, 1.0f));
    return viewPos.xyz;
}

// Convert world normal to view space (no translation)
float3 WorldNormalToViewSpace(float3 worldNormal)
{
    return mul((float3x3)p.stateBuffer.camera.view, worldNormal);
}

//------------------------------------------------------------------------------
// GTAO Implementation (VIEW SPACE like XeGTAO)
//------------------------------------------------------------------------------
float calculateGTAO(uint2 pixel, float3 worldPos, float3 worldNormal, float depth,
                   RWTexture2D<float3> positionBuffer, RWTexture2D<float> depthBuffer,
                   float4 noiseVec, uint2 screenSize)
{
    // Early out for invalid depth
    if (depth <= 0.0f) return 1.0f;

    // Transform to VIEW SPACE (critical!)
    float3 viewPos = WorldToViewSpace(worldPos);
    float3 viewNormal = normalize(WorldNormalToViewSpace(worldNormal));

    // View vector in VIEW SPACE points toward camera (at origin)
    // In view space, camera is at origin, so viewVec = -position normalized
    float3 viewVec = normalize(-viewPos);

    // Calculate screen-space radius (exactly like XeGTAO)
    float viewspaceZ = abs(viewPos.z); // Depth in view space
    float2 pixelSize = 1.0f / float2(screenSize);

    // Extract tanHalfFOV from projection matrix
    // proj[0][0] = 1/tanHalfFOVX, proj[1][1] = 1/tanHalfFOVY
    float tanHalfFOVX = 1.0f / p.stateBuffer.camera.proj[0][0];
    float tanHalfFOVY = 1.0f / p.stateBuffer.camera.proj[1][1];

    // NDCToViewMul (from XeGTAO)
    float2 NDCToViewMul = float2(tanHalfFOVX * 2.0f, tanHalfFOVY * -2.0f);

    // NDCToViewMul_x_PixelSize
    float2 NDCToViewMul_x_PixelSize = NDCToViewMul * pixelSize;

    // Viewspace pixel size at this depth (XeGTAO line 338)
    float2 pixelDirRBViewspaceSizeAtCenterZ = viewspaceZ * NDCToViewMul_x_PixelSize;

    // Screen space radius (XeGTAO line 340)
    float screenRadius = GTAO_RADIUS / pixelDirRBViewspaceSizeAtCenterZ.x;

    // Falloff parameters
    const float effectRadius = GTAO_RADIUS;
    const float falloffRange = GTAO_FALLOFF_RANGE * effectRadius;
    const float falloffFrom = effectRadius * (1.0f - GTAO_FALLOFF_RANGE);
    const float falloffMul = -1.0f / falloffRange;
    const float falloffAdd = falloffFrom / falloffRange + 1.0f;

    // Minimum sample distance
    const float minS = GTAO_PIXEL_TOO_CLOSE_THRESHOLD / screenRadius;

    // Initialize visibility
    float visibility = 0.0f;

    // Fade out for small screen radii (XeGTAO line 343)
    visibility += saturate((10.0f - screenRadius) / 100.0f) * 0.5f;

    // Main GTAO algorithm (following XeGTAO exactly)
    for (int slice = 0; slice < GTAO_SLICES; slice++)
    {
        // R1 sequence noise
        float sliceK = (float(slice) + noiseVec.x) / float(GTAO_SLICES);

        // Lines 5-6 from paper: compute slice angle
        float phi = sliceK * PI; // Note: XeGTAO uses PI not 2*PI
        float cosPhi = cos(phi);
        float sinPhi = sin(phi);
        float2 omega = float2(cosPhi, -sinPhi);

        // Convert to screen pixels
        omega *= screenRadius;

        // Line 8: direction vector IN VIEW SPACE (XY plane, Z=0)
        float3 directionVec = float3(cosPhi, sinPhi, 0);

        // Line 9: orthogonal direction vector
        float3 orthoDirectionVec = directionVec - (dot(directionVec, viewVec) * viewVec);

        // Line 10: axis vector (orthogonal to both)
        float3 axisVec = normalize(cross(orthoDirectionVec, viewVec));

        // Line 11: project normal onto slice plane
        float3 projectedNormalVec = viewNormal - axisVec * dot(viewNormal, axisVec);

        // Line 13-14: sign and length
        float signNorm = sign(dot(orthoDirectionVec, projectedNormalVec));
        float projectedNormalVecLength = length(projectedNormalVec);
        float cosNorm = saturate(dot(projectedNormalVec, viewVec) / max(projectedNormalVecLength, 0.0001f));

        // Line 15: n angle
        float n = signNorm * FastACos(cosNorm);

        // Initialize horizon cosines (XeGTAO uses cosines, not angles!)
        const float lowHorizonCos0 = cos(n + 1.5707963f); // n + PI/2
        const float lowHorizonCos1 = cos(n - 1.5707963f); // n - PI/2
        float horizonCos0 = lowHorizonCos0;
        float horizonCos1 = lowHorizonCos1;

        // Sample along both directions
        for (int step = 0; step < GTAO_STEPS; step++)
        {
            // R1 sequence for step noise
            float stepBaseNoise = float(slice + step * GTAO_STEPS) * 0.6180339887f;
            float stepNoise = frac(noiseVec.y + stepBaseNoise);

            // Line 20: calculate s with noise
            float s = (float(step) + stepNoise) / float(GTAO_STEPS);
            s = pow(s, GTAO_SAMPLE_DISTRIBUTION_POWER);
            s += minS;

            // Lines 21-22: sample offsets
            float2 sampleOffset = s * omega;
            float2 screenPos = float2(pixel);

            // Sample both directions
            float2 samplePixel0 = screenPos + sampleOffset;
            float2 samplePixel1 = screenPos - sampleOffset;

            // Sample position 0 (positive direction)
            if (all(samplePixel0 >= 0) && all(samplePixel0 < screenSize))
            {
                uint2 sampleCoord0 = uint2(samplePixel0);
                float3 sampleWorldPos0 = positionBuffer[sampleCoord0];
                float3 sampleViewPos0 = WorldToViewSpace(sampleWorldPos0);

                float3 sampleDelta0 = sampleViewPos0 - viewPos;
                float sampleDist0 = length(sampleDelta0);

                if (sampleDist0 > 0.01f)
                {
                    // Line 23: horizon vector
                    float3 sampleHorizonVec0 = sampleDelta0 / sampleDist0;

                    // Distance-based falloff
                    float falloffBase0 = length(float3(sampleDelta0.x, sampleDelta0.y, sampleDelta0.z * (1.0f + GTAO_THIN_OCCLUDER_COMPENSATION)));
                    float weight0 = saturate(falloffBase0 * falloffMul + falloffAdd);

                    // Sample horizon cosine (not angle!)
                    float shc0 = dot(sampleHorizonVec0, viewVec);
                    shc0 = lerp(lowHorizonCos0, shc0, weight0);

                    // Track maximum
                    horizonCos0 = max(horizonCos0, shc0);
                }
            }

            // Sample position 1 (negative direction)
            if (all(samplePixel1 >= 0) && all(samplePixel1 < screenSize))
            {
                uint2 sampleCoord1 = uint2(samplePixel1);
                float3 sampleWorldPos1 = positionBuffer[sampleCoord1];
                float3 sampleViewPos1 = WorldToViewSpace(sampleWorldPos1);

                float3 sampleDelta1 = sampleViewPos1 - viewPos;
                float sampleDist1 = length(sampleDelta1);

                if (sampleDist1 > 0.01f)
                {
                    float3 sampleHorizonVec1 = sampleDelta1 / sampleDist1;

                    float falloffBase1 = length(float3(sampleDelta1.x, sampleDelta1.y, sampleDelta1.z * (1.0f + GTAO_THIN_OCCLUDER_COMPENSATION)));
                    float weight1 = saturate(falloffBase1 * falloffMul + falloffAdd);

                    float shc1 = dot(sampleHorizonVec1, viewVec);
                    shc1 = lerp(lowHorizonCos1, shc1, weight1);

                    horizonCos1 = max(horizonCos1, shc1);
                }
            }
        }

        // Fudge factor for slopes (from XeGTAO)
        projectedNormalVecLength = lerp(projectedNormalVecLength, 1.0f, 0.05f);

        // Line 27: calculate horizon angles and integrate
        float h0 = -FastACos(horizonCos1);
        float h1 = FastACos(horizonCos0);

        // Integration formula (exact from XeGTAO)
        float iarc0 = (cosNorm + 2.0f * h0 * sin(n) - cos(2.0f * h0 - n)) / 4.0f;
        float iarc1 = (cosNorm + 2.0f * h1 * sin(n) - cos(2.0f * h1 - n)) / 4.0f;
        float localVisibility = projectedNormalVecLength * (iarc0 + iarc1);

        visibility += localVisibility;
    }

    // Average over slices
    visibility /= float(GTAO_SLICES);

    // Apply final power
    visibility = pow(saturate(visibility), GTAO_FINAL_VALUE_POWER);

    // Prevent total occlusion
    visibility = max(0.03f, visibility);

    return visibility;
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
