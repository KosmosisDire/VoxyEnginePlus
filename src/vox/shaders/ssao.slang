#include "shared.inl"

[[vk::push_constant]]
const ComputePush p;

// SSAO settings
static const int KERNEL_SIZE = 16;
static const float RADIUS = 0.8;
static const float BIAS = 0.025;
static const float MAX_DISTANCE = 2.0;
static const float FALLOFF = 0.001;
static const float STRENGTH = 1.0;

// Better random vector generation
float3 hash3(float2 p)
{
    float3 p3 = frac(float3(p.xyx) * float3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yxz + 19.19);
    p3 = frac(float3(p3.x * p3.y, p3.x * p3.z, p3.y * p3.z));
    return p3 * 2.0 - 1.0;
}

float3 hemispherePoint(float u1, float u2)
{
    float phi = 2.0 * 3.14159265 * u1;
    float cosTheta = sqrt(1.0 - u2);
    float sinTheta = sqrt(u2);
    return float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

[numthreads(8, 8, 1)]
void main(uint3 DTid: SV_DispatchThreadID)
{
    uint2 pixel = DTid.xy;
    if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
        return;

    RWTexture2D<float4> position = RWTexture2D<float4>::get(p.gbuffer.position);
    RWTexture2D<int4> normal = RWTexture2D<int4>::get(p.gbuffer.normal);
    RWTexture2D<float> ssao = RWTexture2D<float>::get(p.gbuffer.ssao);
    RWTexture2D<float4> blueNoiseTex = RWTexture2D<float4>::get(p.blueNoise);

    float3 pos = position[pixel * 2].xyz;
    float3 norm = normalize(float3(normal[pixel * 2].xyz) / 127.0f);

    // Skip background pixels
    if (all(pos == 0.0))
    {
        ssao[pixel] = 1.0;
        return;
    }

    float occlusion = 0.0;

    // Use blue noise for rotation
    float4 blueNoise = blueNoiseTex[pixel % 128];
    float3 randomVec = normalize(blueNoise.xyz * 2.0 - 1.0);

    float3 tangent = normalize(randomVec - norm * dot(randomVec, norm));
    float3 bitangent = cross(norm, tangent);
    float3x3 TBN = float3x3(tangent, bitangent, norm);

    for (int i = 0; i < KERNEL_SIZE; i++)
    {
        // Use blue noise for sample distribution
        float2 noiseOffset = blueNoise.xy * 2.0 - 1.0;
        float r = float(i) / float(KERNEL_SIZE);
        float phi = (r + noiseOffset.x * 0.1) * (PI * 2.0) * 0.618034;

        float cosTheta = 1.0 - (r + noiseOffset.y * 0.1);
        float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

        float3 sample = float3(
            cos(phi) * sinTheta,
            sin(phi) * sinTheta,
            cosTheta);

        // Randomize sample length
        float scale = r * r;
        sample *= lerp(0.1, 1.0, scale) * RADIUS;

        // Transform sample to view space
        sample = mul(TBN, sample);
        float3 samplePos = pos + sample;

        // Project sample position
        float4 offset = mul(p.state_ptr.camera.viewProj, float4(samplePos, 1.0));
        if (offset.w <= 0.0)
            continue; // Behind camera

        offset.xyz /= offset.w;
        if (any(abs(offset.xyz) > 1.0))
            continue; // Outside frustum

        offset.xyz = offset.xyz * 0.5 + 0.5;
        uint2 samplePixel = uint2(offset.xy * p.frame_dim);

        // Validate sample position
        if (any(samplePixel >= p.frame_dim))
            continue;

        // Get sample position
        float4 samplePos4D = position[samplePixel];
        if (all(samplePos4D.xyz == 0.0))
            continue;

        // Calculate occlusion contribution
        float3 v = samplePos4D.xyz - pos;
        float vv = dot(v, v);
        float vn = dot(normalize(v + float3(0.0001, 0.0001, 0.0001)), norm);

        // Distance-based falloff
        float distanceFalloff = 1.0 / (1.0 + vv * (1.0 / (RADIUS * RADIUS)));

        // Angle-based weighting
        float angleFalloff = smoothstep(-1.0, 1.0, vn);

        // Range and horizon checks
        float rangeCheck = smoothstep(MAX_DISTANCE, 0.0, sqrt(vv));
        float horizonCheck = vn > BIAS ? 1.0 : 0.0;

        occlusion += distanceFalloff * angleFalloff * rangeCheck * horizonCheck;
    }

    occlusion = 1.0 - (occlusion / float(KERNEL_SIZE));
    occlusion = pow(saturate(occlusion), 2.0);  // Enhance contrast
    occlusion = lerp(1.0, occlusion, STRENGTH); // Control strength

    ssao[pixel] = occlusion;
}
