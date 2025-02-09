#include "atmosphere.slang"
#include "lygia/color/tonemap/unreal.hlsl"
#include "raycast.slang"
#include "voxel-hashmap.slang"

static const float DEPTH_SCALE = 0.00002;

float3 RGBtoHSL(float3 color)
{
    float maxColor = max(max(color.r, color.g), color.b);
    float minColor = min(min(color.r, color.g), color.b);
    float delta = maxColor - minColor;

    float3 hsl = float3(0, 0, (maxColor + minColor) / 2.0f);

    if (delta != 0.0f)
    {
        hsl.y = hsl.z < 0.5f ? delta / (maxColor + minColor) : delta / (2.0f - maxColor - minColor);

        float deltaR = (((maxColor - color.r) / 6.0f) + (delta / 2.0f)) / delta;
        float deltaG = (((maxColor - color.g) / 6.0f) + (delta / 2.0f)) / delta;
        float deltaB = (((maxColor - color.b) / 6.0f) + (delta / 2.0f)) / delta;

        if (color.r == maxColor)
            hsl.x = deltaB - deltaG;
        else if (color.g == maxColor)
            hsl.x = (1.0f / 3.0f) + deltaR - deltaB;
        else if (color.b == maxColor)
            hsl.x = (2.0f / 3.0f) + deltaG - deltaR;

        if (hsl.x < 0.0f)
            hsl.x += 1.0f;
        else if (hsl.x > 1.0f)
            hsl.x -= 1.0f;
    }

    return hsl;
}

float3 HSLtoRGB(float3 hsl)
{
    float3 rgb;

    if (hsl.y == 0.0f)
    {
        rgb = float3(hsl.z, hsl.z, hsl.z);
    }
    else
    {
        float q = hsl.z < 0.5f ? hsl.z * (1.0f + hsl.y) : hsl.z + hsl.y - (hsl.z * hsl.y);
        float p = 2.0f * hsl.z - q;

        float hk = hsl.x;
        float3 tk;
        tk.r = hk + 1.0f / 3.0f;
        tk.g = hk;
        tk.b = hk - 1.0f / 3.0f;

        for (int i = 0; i < 3; i++)
        {
            if (tk[i] < 0.0f)
                tk[i] += 1.0f;
            else if (tk[i] > 1.0f)
                tk[i] -= 1.0f;

            if (tk[i] * 6.0f < 1.0f)
                rgb[i] = p + ((q - p) * 6.0f * tk[i]);
            else if (tk[i] * 2.0f < 1.0f)
                rgb[i] = q;
            else if (tk[i] * 3.0f < 2.0f)
                rgb[i] = p + ((q - p) * 6.0f * (2.0f / 3.0f - tk[i]));
            else
                rgb[i] = p;
        }
    }

    return rgb;
}

float3 random(uint seed)
{
    uint seed1 = seed * 747796405u + 2891336453u;
    uint seed2 = ((seed + 1) * 747796405u + 2891336453u);
    uint seed3 = ((seed + 2) * 747796405u + 2891336453u);

    seed1 = ((seed1 >> ((seed1 >> 28) + 4)) ^ seed1) * 277803737u;
    seed2 = ((seed2 >> ((seed2 >> 28) + 4)) ^ seed2) * 277803737u;
    seed3 = ((seed3 >> ((seed3 >> 28) + 4)) ^ seed3) * 277803737u;

    seed1 = (seed1 >> 22) ^ seed1;
    seed2 = (seed2 >> 22) ^ seed2;
    seed3 = (seed3 >> 22) ^ seed3;

    return float3(seed1, seed2, seed3) / 4294967296.0;
}

[numthreads(8, 8, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    uint2 pixel = pixel_i.xy;
    if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
        return;

    RWTexture2D<float4> render_image = RWTexture2D<float4>::get(p.final_image);

    // Read from gbuffer textures
    RWTexture2D<float3> albedoTex = RWTexture2D<float3>::get(p.gbuffer.color);
    RWTexture2D<int3> normalTex = RWTexture2D<int3>::get(p.gbuffer.normal);
    RWTexture2D<float3> positionTex = RWTexture2D<float3>::get(p.gbuffer.position);
    RWTexture2D<float2> voxelUVsTex = RWTexture2D<float2>::get(p.gbuffer.voxelUVs);
    RWTexture2D<float2> brickUVsTex = RWTexture2D<float2>::get(p.gbuffer.brickUVs);
    RWTexture2D<float3> indirectTex = RWTexture2D<float3>::get(p.gbuffer.indirect);
    RWTexture2D<float3> indirectPerVoxPassTex = RWTexture2D<float3>::get(p.gbuffer.indirectPerVoxelPass2);
    RWTexture2D<float2> motionTex = RWTexture2D<float2>::get(p.gbuffer.motion);
    RWTexture2D<float> ssaoTex = RWTexture2D<float>::get(p.gbuffer.ssao);
    RWTexture2D<float> depthTex = RWTexture2D<float>::get(p.gbuffer.depth);
    RWTexture2D<float> depthHalfResTex = RWTexture2D<float>::get(p.gbuffer.depthHalfRes);
    RWTexture2D<uint2> voxelIDTex = RWTexture2D<uint2>::get(p.gbuffer.voxelIDs);
    RWTexture2D<uint> voxelFaceIDTex = RWTexture2D<uint>::get(p.gbuffer.voxelFaceIDs);
    RWTexture2D<uint> materialIDTex = RWTexture2D<uint>::get(p.gbuffer.materialIDs);
    RWTexture2D<float> shadowTex = RWTexture2D<float>::get(p.gbuffer.shadow);
    RWTexture2D<float3> blueNoiseTex = RWTexture2D<float3>::get(p.blueNoise);

    // Sample all buffers at current pixel
    float3 albedo = albedoTex[pixel];
    int3 normal = normalTex[pixel];
    float3 position = positionTex[pixel].xyz;
    float2 voxelUV = voxelUVsTex[pixel];
    float2 brickUV = brickUVsTex[pixel];
    float3 pixelLight = indirectTex[pixel / 2];
    float3 perVoxAvg = indirectPerVoxPassTex[pixel / 2];
    float2 motion = motionTex[pixel];
    float ssao = ssaoTex[pixel / 2];
    float depth = depthTex[pixel];
    float depthHalfRes = depthHalfResTex[pixel / 2];
    uint2 voxelID = voxelIDTex[pixel]; // round to account for half res sampling
    uint voxelFaceID = voxelFaceIDTex[pixel];
    uint materialID = materialIDTex[pixel];
    materialID = clamp(materialID, 0, TOTAL_MATERIALS - 1);
    Material material = p.materials_ptr.materials[materialID];
    float shadow = shadowTex[pixel];

    float hashtableCollisionFactor = get_hashtable_collision_factor(p.voxel_hashmap_ptr, voxelID.x, voxelFaceID) / 10.0;

    uint slot = get_hashtable_index(p.voxel_hashmap_ptr, voxelID.x, voxelFaceID);
    var entry = p.voxel_hashmap_ptr.entries[slot];
    float3 voxelLight = entry.lighting;
    int samples = entry.historyCount;

    // if slot is empty then sample 4 adjacent pixels and average values from them (as long as they aren't empty)
    if (slot == EMPTY_KEY)
    {
        float3 avgLight = float3(0, 0, 0);
        int avgSamples = 0;

        for (int y = -1; y <= 1; y++)
        {
            for (int x = -1; x <= 1; x++)
            {
                int2 offset = int2(pixel) + int2(x, y);
                if (offset.x < 0 || offset.x >= p.frame_dim.x || offset.y < 0 || offset.y >= p.frame_dim.y)
                    continue;

                uint offsetSlot = get_hashtable_index(p.voxel_hashmap_ptr, voxelIDTex[offset].x, voxelFaceIDTex[offset]);
                if (offsetSlot != EMPTY_KEY)
                {
                    avgLight += p.voxel_hashmap_ptr.entries[offsetSlot].lighting;
                    avgSamples++;
                }
            }
        }

        if (avgSamples > 0)
        {
            voxelLight = avgLight / avgSamples;
            samples = avgSamples;
        }
    }

    float3 hslLight = RGBtoHSL(voxelLight);
    hslLight.y = hslLight.y * 1.4;
    voxelLight = HSLtoRGB(hslLight);

    // Get ray and atmosphere setup
    float3 rayOrigin, rayDir;
    getRayFromPixel(pixel, p.frame_dim, p.state_ptr.camera, rayOrigin, rayDir);
    float3 sunDir = p.state_ptr.sunDir;
    AtmosphereParams atmosphere = initAtmosphere(rayDir, sunDir);

    // Handle background pixels (sky)
    if (depth > 1000.0f)
    {
        render_image[pixel] = tonemapUnreal(float4(getAtmosphereColor(atmosphere, p.state_ptr.time), 1.0));
        return;
    }

    // beveled normal using box filter
    float3 normalSum = float3(0, 0, 0);
    int normalCount = 0;
    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            int2 offset = int2(pixel) + int2(x, y);
            if (offset.x < 0 || offset.x >= p.frame_dim.x || offset.y < 0 || offset.y >= p.frame_dim.y)
                continue;

            int3 offsetNormal = normalTex[offset];
            if (any(offsetNormal != int3(0, 0, 0)))
            {
                normalSum += float3(offsetNormal);
                normalCount++;
            }
        }
    }

    float3 bevNormal = normalSum / normalCount;
    bevNormal += random(voxelFaceID + voxelID.x * 216) * 0.05;

    // Get sun color from atmosphere for lighting
    float3 lightColor = getSunColor(atmosphere);
    float3 baseSkyColor = getBaseSkyColor(atmosphere);
    float ldot = max(dot(bevNormal, sunDir), 0.0);

    voxelLight += lightColor * ldot * shadow;
    voxelLight *= (shadow + 1) / 2.0;

    // Specular lighting
    float3 viewDir = normalize(rayOrigin - position);
    float3 halfVec = normalize(viewDir + sunDir);
    float3 specular = pow(max(dot(bevNormal, halfVec), 0.0), 32);
    specular *= getBaseSkyColor(initAtmosphere(viewDir, sunDir));
    specular *= shadow;

    float3 finalColor = voxelLight * material.albedo + material.emission + specular;

    // Fog effect
    // float fogFactor = saturate(depthValue * depthValue * DEPTH_SCALE);
    // finalColor = lerp(finalColor, baseSkyColor, fogFactor);
    // finalColor *= ssaoValue;

    // finalColor = voxelLight;
    // finalColor = pixelLight;
    // finalColor = perVoxAvg;
    // finalColor = albedoColor.rgb;
    // finalColor = (abs(motionVector.xy) * p.frame_dim * 0.1).xyx;
    // finalColor = pow(finalColor, 5);
    // finalColor = float3((float2)pixel.xy / p.frame_dim.xy, 0);
    // finalColor = float3((float2)pixel.xy / p.frame_dim.xy, 0);
    // finalColor = float3(ssaoValue, ssaoValue, ssaoValue);
    // finalColor = float3(depthValue, depthValue, depthValue);
    // finalColor = float3(depthValueHalfRes, depthValueHalfRes, depthValueHalfRes);
    // finalColor = float3(voxelID.x, voxelID.y, 0) / 1000.0;
    // finalColor = float3(voxelFaceID, voxelFaceID, voxelFaceID) / 1000.0;
    // finalColor = float3(0, voxelIDLast.y, 0) / 1000.0;
    // finalColor = float3(materialID, materialID, materialID) / 16.0;
    // finalColor = float3(shadowValue, shadowValue, shadowValue);
    // finalColor = float3(samples, samples, samples) / 100.0;
    // finalColor = float3(hitCount, hitCount, hitCount);
    // finalColor = float3(ssaoValue, ssaoValue, ssaoValue);
    // finalColor = float3(voxelUV.x, voxelUV.y, 0);
    // finalColor = float3(brickUV.x, brickUV.y, 0);
    // finalColor = specular;
    // finalColor = float3(hashtableCollisionFactor);

    // Apply firefly removal after all other processing but before final output
    // finalColor = removeFireflies(finalColor, pixel, render_image);

    // post process
    finalColor *= 2.5;
    finalColor = tonemapUnreal(finalColor);

    // convert to grayscale
    // finalColor = float3(dot(finalColor, float3(0.299, 0.587, 0.114)));

    // Debug visualization modes
    if ((bool)p.state_ptr.settings.showNormals)
        finalColor = bevNormal;
    if ((bool)p.state_ptr.settings.showHitPos)
        finalColor = position / float(GRID_SIZE);
    if ((bool)p.state_ptr.settings.showDepth)
        finalColor = float3(depth / 100.0);
    if ((bool)p.state_ptr.settings.showDepthPrepass)
        finalColor = float3(depthHalfRes / 100.0);

    render_image[pixel] = float4(finalColor, 1.0);
}
