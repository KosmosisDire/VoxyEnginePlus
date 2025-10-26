#include "common.slang"
#include "tracing/trace-contree.slang"

//------------------------------------------------------------------------------
// GTAO Configuration
//------------------------------------------------------------------------------
static const int GTAO_SLICES = 9;
static const int GTAO_STEPS = 9;
static const float GTAO_RADIUS = 10.0f;
static const float GTAO_FALLOFF_RANGE = 0.8f;
static const float GTAO_SAMPLE_DISTRIBUTION_POWER = 1.0f;
static const float GTAO_THIN_OCCLUDER_COMPENSATION = 1.5f;
static const float GTAO_FINAL_VALUE_POWER = 1.0f;
static const float GTAO_PIXEL_TOO_CLOSE_THRESHOLD = 0.5f;

float FastACos(float inX)
{
    const float PI = 3.141593f;
    const float HALF_PI = 1.570796f;
    float x = abs(inX);
    float res = -0.156583f * x + HALF_PI;
    res *= sqrt(1.0f - x);
    return (inX >= 0) ? res : PI - res;
}

float3 WorldToViewSpace(float3 worldPos)
{
    float4 viewPos = mul(p.stateBuffer.camera.view, float4(worldPos, 1.0f));
    return viewPos.xyz;
}

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
    if (depth <= 0.0f) return 1.0f;

    // In view space, camera is at origin
    float3 viewPos = WorldToViewSpace(worldPos);
    float3 viewNormal = normalize(WorldNormalToViewSpace(worldNormal));
    float3 viewVec = normalize(-viewPos);

    float viewspaceZ = abs(viewPos.z);
    float2 pixelSize = 1.0f / float2(screenSize);

    // Extract tanHalfFOV from projection matrix (proj[i][i] = 1/tanHalfFOV)
    float tanHalfFOVX = 1.0f / p.stateBuffer.camera.proj[0][0];
    float tanHalfFOVY = 1.0f / p.stateBuffer.camera.proj[1][1];

    float2 NDCToViewMul = float2(tanHalfFOVX * 2.0f, tanHalfFOVY * -2.0f);
    float2 pixelDirRBViewspaceSizeAtCenterZ = viewspaceZ * (NDCToViewMul * pixelSize);
    float screenRadius = GTAO_RADIUS / pixelDirRBViewspaceSizeAtCenterZ.x;

    const float effectRadius = GTAO_RADIUS;
    const float falloffRange = GTAO_FALLOFF_RANGE * effectRadius;
    const float falloffFrom = effectRadius * (1.0f - GTAO_FALLOFF_RANGE);
    const float falloffMul = -1.0f / falloffRange;
    const float falloffAdd = falloffFrom / falloffRange + 1.0f;
    const float minS = GTAO_PIXEL_TOO_CLOSE_THRESHOLD / screenRadius;

    float visibility = saturate((10.0f - screenRadius) / 100.0f) * 0.5f;

    for (int slice = 0; slice < GTAO_SLICES; slice++)
    {
        float sliceK = (float(slice) + noiseVec.x) / float(GTAO_SLICES);

        // XeGTAO uses PI (not 2*PI) for slice angle computation
        float phi = sliceK * PI;
        float cosPhi = cos(phi);
        float sinPhi = sin(phi);
        float2 omega = float2(cosPhi, -sinPhi) * screenRadius;

        float3 directionVec = float3(cosPhi, sinPhi, 0);
        float3 orthoDirectionVec = directionVec - (dot(directionVec, viewVec) * viewVec);
        float3 axisVec = normalize(cross(orthoDirectionVec, viewVec));
        float3 projectedNormalVec = viewNormal - axisVec * dot(viewNormal, axisVec);

        float signNorm = sign(dot(orthoDirectionVec, projectedNormalVec));
        float projectedNormalVecLength = length(projectedNormalVec);
        float cosNorm = saturate(dot(projectedNormalVec, viewVec) / max(projectedNormalVecLength, 0.0001f));
        float n = signNorm * FastACos(cosNorm);

        // XeGTAO stores horizon cosines, not angles
        const float lowHorizonCos0 = cos(n + 1.5707963f);
        const float lowHorizonCos1 = cos(n - 1.5707963f);
        float horizonCos0 = lowHorizonCos0;
        float horizonCos1 = lowHorizonCos1;

        for (int step = 0; step < GTAO_STEPS; step++)
        {
            float stepBaseNoise = float(slice + step * GTAO_STEPS) * 0.6180339887f;
            float stepNoise = frac(noiseVec.y + stepBaseNoise);

            float s = (float(step) + stepNoise) / float(GTAO_STEPS);
            s = pow(s, GTAO_SAMPLE_DISTRIBUTION_POWER) + minS;
            float2 sampleOffset = s * omega;
            float2 screenPos = float2(pixel);

            float2 samplePixel0 = screenPos + sampleOffset;
            float2 samplePixel1 = screenPos - sampleOffset;

            if (all(samplePixel0 >= 0) && all(samplePixel0 < screenSize))
            {
                uint2 sampleCoord0 = uint2(samplePixel0);
                float3 sampleViewPos0 = WorldToViewSpace(positionBuffer[sampleCoord0]);
                float3 sampleDelta0 = sampleViewPos0 - viewPos;
                float sampleDist0 = length(sampleDelta0);

                if (sampleDist0 > 0.01f)
                {
                    float3 sampleHorizonVec0 = sampleDelta0 / sampleDist0;
                    float falloffBase0 = length(float3(sampleDelta0.x, sampleDelta0.y, sampleDelta0.z * (1.0f + GTAO_THIN_OCCLUDER_COMPENSATION)));
                    float weight0 = saturate(falloffBase0 * falloffMul + falloffAdd);
                    float shc0 = lerp(lowHorizonCos0, dot(sampleHorizonVec0, viewVec), weight0);
                    horizonCos0 = max(horizonCos0, shc0);
                }
            }

            if (all(samplePixel1 >= 0) && all(samplePixel1 < screenSize))
            {
                uint2 sampleCoord1 = uint2(samplePixel1);
                float3 sampleViewPos1 = WorldToViewSpace(positionBuffer[sampleCoord1]);
                float3 sampleDelta1 = sampleViewPos1 - viewPos;
                float sampleDist1 = length(sampleDelta1);

                if (sampleDist1 > 0.01f)
                {
                    float3 sampleHorizonVec1 = sampleDelta1 / sampleDist1;
                    float falloffBase1 = length(float3(sampleDelta1.x, sampleDelta1.y, sampleDelta1.z * (1.0f + GTAO_THIN_OCCLUDER_COMPENSATION)));
                    float weight1 = saturate(falloffBase1 * falloffMul + falloffAdd);
                    float shc1 = lerp(lowHorizonCos1, dot(sampleHorizonVec1, viewVec), weight1);
                    horizonCos1 = max(horizonCos1, shc1);
                }
            }
        }

        // Fudge factor for slopes (from XeGTAO)
        projectedNormalVecLength = lerp(projectedNormalVecLength, 1.0f, 0.05f);

        float h0 = -FastACos(horizonCos1);
        float h1 = FastACos(horizonCos0);

        float iarc0 = (cosNorm + 2.0f * h0 * sin(n) - cos(2.0f * h0 - n)) / 4.0f;
        float iarc1 = (cosNorm + 2.0f * h1 * sin(n) - cos(2.0f * h1 - n)) / 4.0f;
        visibility += projectedNormalVecLength * (iarc0 + iarc1);
    }

    visibility /= float(GTAO_SLICES);
    visibility = pow(saturate(visibility), GTAO_FINAL_VALUE_POWER);
    visibility = max(0.03f, visibility);

    return visibility;
}

//------------------------------------------------------------------------------
// Main Shader
//------------------------------------------------------------------------------
[numthreads(8, 8, 1)]
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
        uint threadIndex = group_thread_id.x + group_thread_id.y * 8;

        var ray = Ray.FromPixel(pixel, p.screenSize, p.stateBuffer.camera);
        ray.maxIterations = 200;

        var hit = traceContree(ray, threadIndex);

        if (!hit.hit)
        {
            colorTex[pixel] = float3(0.5f, 0.7f, 1.0f);
            normalTex[pixel] = 0;
            positionTex[pixel] = 0;
            depthTex[pixel] = -1;
            return;
        }

        float3 worldNormal = hit.normal;
        float3 worldPos = hit.position;

        normalTex[pixel] = worldNormal;
        positionTex[pixel] = worldPos;
        depthTex[pixel] = hit.distance;

        // Get material properties from material ID
        uint materialId = hit.materialId;

        // Simple material color lookup
        float3 albedoColor;
        if (materialId == 0) {
            albedoColor = float3(0.5, 0.5, 0.5); // Default gray
        } else if (materialId == 1) {
            albedoColor = float3(0.3, 0.7, 0.2); // Grass green
        } else if (materialId == 2) {
            albedoColor = float3(0.6, 0.5, 0.4); // Stone gray-brown
        } else if (materialId == 3) {
            albedoColor = float3(0.5, 0.3, 0.2); // Dirt brown
        }

        float3 lightDir = normalize(p.stateBuffer.sunDir);

        Ray shadowRay;
        shadowRay.origin = worldPos + worldNormal * 0.001f;
        shadowRay.direction = lightDir;
        shadowRay.maxDist = 1000.0f;
        shadowRay.maxIterations = 200;

        var shadowHit = traceContree(shadowRay, threadIndex);

        // Shadows affect direct lighting differently:
        // - Diffuse: partial occlusion (ambient bounce light still reaches surface)
        // - Specular: full occlusion (no direct light = no specular highlight)
        float diffuseShadow = shadowHit.hit ? 0.0f : 1.0f;  // Soft shadow for diffuse
        float specularShadow = shadowHit.hit ? 0.0f : 1.0f; // Hard shadow for specular

        float ndotl = max(dot(worldNormal, lightDir), 0.0f);
        float3 diffuse = albedoColor * ndotl * diffuseShadow;

        float3 viewDir = normalize(p.stateBuffer.camera.position - worldPos);
        float3 halfDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(worldNormal, halfDir), 0.0f), 32.0f);
        float3 specular = float3(1.0f, 1.0f, 1.0f) * 0.2f * spec * specularShadow;

        float3 ambient = albedoColor * 0.2f;

        float skyFactor = worldNormal.y * 0.5f + 0.5f;
        float3 skyColor = float3(0.5f, 0.7f, 1.0f) * 0.0f;
        float3 skyLight = albedoColor * skyColor * skyFactor;

        float3 finalColor = diffuse + specular + ambient + skyLight;
        colorTex[pixel] = finalColor;
    }
    // Pass 1: Calculate GTAO from complete GBuffer
    else if (p.passNum == 1)
    {
        float depth = depthTex[pixel];
        if (depth < 0)
            return;

        float3 worldPos = positionTex[pixel];
        float3 worldNormal = normalTex[pixel];

        float4 noiseVec = blueNoise[pixel % 128];
        float ao = calculateGTAO(pixel, worldPos, worldNormal, depth, positionTex, depthTex, noiseVec, p.screenSize);
        aoTex[pixel] = ao;
    }
    // Pass 2: Apply AO to all pixels with specular occlusion
    else if (p.passNum == 2)
    {
        float depth = depthTex[pixel];
        if (depth < 0)
            return;

        float3 color = colorTex[pixel];
        float3 worldPos = positionTex[pixel];
        float3 worldNormal = normalTex[pixel];

        // Read full-resolution AO directly
        float ao = aoTex[pixel];

        // Calculate specular occlusion with view-dependent falloff
        // Based on the horizon-based specular occlusion technique
        float3 viewDir = normalize(p.stateBuffer.camera.position - worldPos);
        float ndotv = saturate(dot(worldNormal, viewDir));

        // Specular roughness (0 = mirror, 1 = rough)
        // Currently hardcoded to match the spec power of 32 in pass 0
        float roughness = sqrt(2.0f / (32.0f + 2.0f)); // Convert from specular power to roughness

        // Specular occlusion: smoother surfaces get less occlusion in grazing angles
        // This approximates the micro-occlusion behavior of rough vs smooth surfaces
        float specularOcclusion = saturate(pow(ndotv + ao, roughness) - 1.0f + ao);

        // Apply different occlusion to diffuse vs specular components
        // Approximate split: diffuse = 70%, specular = 30% of the total lighting
        // This is a rough approximation since we don't have separated lighting channels
        float diffuseWeight = 0.7f;
        float specularWeight = 0.3f;

        float combinedOcclusion = diffuseWeight * ao + specularWeight * specularOcclusion;

        colorTex[pixel] = color * combinedOcclusion;
    }
}
