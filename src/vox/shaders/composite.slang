#include "atmosphere.slang"
#include "lygia/color/tonemap/unreal.hlsl"
#include "raycast.slang"
#include "voxel-hashmap.slang"

static const float DEPTH_SCALE = 0.00002;

[numthreads(8, 8, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    uint2 pixel = pixel_i.xy;
    if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
        return;

    RWTexture2D<float4> render_image = RWTexture2D<float4>::get(p.final_image);

    // Read from gbuffer textures
    RWTexture2D<float3> albedo = RWTexture2D<float3>::get(p.gbuffer.color);
    RWTexture2D<int3> normal = RWTexture2D<int3>::get(p.gbuffer.normal);
    RWTexture2D<float3> position = RWTexture2D<float3>::get(p.gbuffer.position);
    RWTexture2D<float2> voxelUVs = RWTexture2D<float2>::get(p.gbuffer.voxelUVs);
    RWTexture2D<float2> brickUVs = RWTexture2D<float2>::get(p.gbuffer.brickUVs);
    RWTexture2D<float3> indirect = RWTexture2D<float3>::get(p.gbuffer.indirect);
    RWTexture2D<float3> indirectPerVoxPass = RWTexture2D<float3>::get(p.gbuffer.indirectPerVoxelPass1);
    RWTexture2D<float2> motion = RWTexture2D<float2>::get(p.gbuffer.motion);
    RWTexture2D<float> ssao = RWTexture2D<float>::get(p.gbuffer.ssao);
    RWTexture2D<float> depth = RWTexture2D<float>::get(p.gbuffer.depth);
    RWTexture2D<float> depthHalfRes = RWTexture2D<float>::get(p.gbuffer.depthHalfRes);
    RWTexture2D<int2> voxelIDs = RWTexture2D<int2>::get(p.gbuffer.voxelIDs);
    RWTexture2D<int2> voxelIDsLast = RWTexture2D<int2>::get(p.gbuffer.voxelIDsLast);
    RWTexture2D<uint> materialIDs = RWTexture2D<uint>::get(p.gbuffer.materialIDs);
    RWTexture2D<float> shadow = RWTexture2D<float>::get(p.gbuffer.shadow);

    // Sample all buffers at current pixel
    float3 albedoColor = albedo[pixel];
    int3 normalVector = normal[pixel];
    float3 worldPos = position[pixel].xyz;
    float2 voxelUV = voxelUVs[pixel];
    float2 brickUV = brickUVs[pixel];
    float3 pixelLight = indirect[pixel / 2];
    float3 perVoxAvg = indirectPerVoxPass[pixel / 2];
    float2 motionVector = motion[pixel];
    float ssaoValue = ssao[pixel / 2];
    float depthValue = depth[pixel];
    float depthValueHalfRes = depthHalfRes[pixel / 2];
    int2 voxelID = voxelIDs[pixel]; // round to account for half res sampling
    int2 voxelIDLast = voxelIDsLast[pixel];
    uint materialID = materialIDs[pixel];
    materialID = clamp(materialID, 0, TOTAL_MATERIALS - 1);
    Material material = p.materials_ptr.materials[materialID];
    float shadowValue = shadow[pixel];

    uint slot = get_hashtable_index(p.voxel_hashmap_ptr, voxelID.x, voxelID.y);
    int hitCount = p.state_ptr.frame - p.past_voxel_hashmap_ptr.hitCounts[slot];
    float3 voxelLight = p.voxel_hashmap_ptr.indirectLight[slot];
    int samples = p.voxel_hashmap_ptr.temporalFrames[slot];

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

                uint offsetSlot = get_hashtable_index(p.voxel_hashmap_ptr, voxelIDs[offset].x, voxelIDs[offset].y);
                if (offsetSlot != EMPTY_KEY)
                {
                    avgLight += p.voxel_hashmap_ptr.indirectLight[offsetSlot];
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

    // Get ray and atmosphere setup
    float3 rayOrigin, rayDir;
    getRayFromPixel(pixel, p.frame_dim, p.state_ptr.camera, rayOrigin, rayDir);
    float3 sunDir = p.state_ptr.sunDir;
    AtmosphereParams atmosphere = initAtmosphere(rayDir, sunDir);

    // Handle background pixels (sky)
    if (depthValue > 1000.0f)
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

            int3 offsetNormal = normal[offset];
            if (any(offsetNormal != int3(0, 0, 0)))
            {
                normalSum += float3(offsetNormal);
                normalCount++;
            }
        }
    }

    float3 bevNormal = normalSum / normalCount;

    // Get sun color from atmosphere for lighting
    float3 lightColor = getSunColor(atmosphere);
    float3 baseSkyColor = getBaseSkyColor(atmosphere);
    float ldot = max(dot(bevNormal, sunDir), 0.0);

    float3 viewDir = normalize(rayOrigin - worldPos);
    float3 halfVec = normalize(viewDir + sunDir);
    float3 specular = pow(max(dot(bevNormal, halfVec), 0.0), 32);
    specular *= getBaseSkyColor(initAtmosphere(viewDir, sunDir));
    specular *= 1 - shadowValue;

    if (!shadowValue)
    {
        voxelLight += lightColor * ldot;
    }

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
    // finalColor = float3(0, voxelIDLast.y, 0) / 1000.0;
    // finalColor = float3(materialID, materialID, materialID) / 16.0;
    // finalColor = float3(shadowValue, shadowValue, shadowValue);
    // finalColor = float3(samples, samples, samples) / 100.0;
    // finalColor = float3(hitCount, hitCount, hitCount);
    // finalColor = float3(ssaoValue, ssaoValue, ssaoValue);
    // finalColor = float3(voxelUV.x, voxelUV.y, 0);
    // finalColor = float3(brickUV.x, brickUV.y, 0);
    // finalColor = specular;

    // Apply firefly removal after all other processing but before final output
    // finalColor = removeFireflies(finalColor, pixel, render_image);

    // post process
    finalColor = tonemapUnreal(finalColor);

    // Debug visualization modes
    if ((bool)p.state_ptr.settings.showNormals)
        finalColor = bevNormal;
    if ((bool)p.state_ptr.settings.showHitPos)
        finalColor = worldPos / float(GRID_SIZE);
    if ((bool)p.state_ptr.settings.showDepth)
        finalColor = float3(depthValue / 100.0);
    if ((bool)p.state_ptr.settings.showDepthPrepass)
        finalColor = float3(depthValueHalfRes / 100.0);

    render_image[pixel] = float4(finalColor, 1.0);
}
