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
    RWTexture2D<float3> normal = RWTexture2D<float3>::get(p.gbuffer.normal);
    RWTexture2D<float3> position = RWTexture2D<float3>::get(p.gbuffer.position);
    RWTexture2D<float3> indirect = RWTexture2D<float3>::get(p.gbuffer.indirectDenoised);
    RWTexture2D<float2> motion = RWTexture2D<float2>::get(p.gbuffer.motion);
    RWTexture2D<float> ssao = RWTexture2D<float>::get(p.gbuffer.ssao);
    RWTexture2D<float> depth = RWTexture2D<float>::get(p.gbuffer.depth);
    RWTexture2D<float> depthHalfRes = RWTexture2D<float>::get(p.gbuffer.depthHalfRes);
    RWTexture2D<int2> voxelIDs = RWTexture2D<int2>::get(p.gbuffer.voxelIDs);

    // Sample all buffers at current pixel
    float3 albedoColor = albedo[pixel];
    float3 normalVector = normal[pixel];
    float3 worldPos = position[pixel].xyz;
    float3 pixelLight = indirect[pixel / 2];
    float2 motionVector = motion[pixel];
    float ssaoValue = ssao[pixel / 2];
    float depthValue = depth[pixel];
    float depthValueHalfRes = depthHalfRes[pixel / 2];
    int2 voxelID = voxelIDs[pixel];

    uint slot = hashVoxel(voxelID.x, voxelID.y);

    uint32_t tries = 0;
    while (tries < 100)
    {
        if (p.voxel_hashmap_ptr.brickIndices[slot] == voxelID.x && p.voxel_hashmap_ptr.voxelIndices[slot] == voxelID.y)
        {
            break;
        }
        if (p.voxel_hashmap_ptr.brickIndices[slot] == EMPTY_KEY && p.voxel_hashmap_ptr.voxelIndices[slot] == EMPTY_KEY)
        {
            slot = EMPTY_KEY;
            break;
        }

        slot = (slot + 1) & (HASH_TABLE_SIZE - 1);
        tries++;
    }

    float3 voxelLight = p.voxel_hashmap_ptr.indirectLight[slot] / max(p.voxel_hashmap_ptr.hitCounts[slot], 1);
    int samples = p.voxel_hashmap_ptr.temporalFrames[slot];

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

    // Get sun color from atmosphere for lighting
    float3 lightColor = getSunColor(atmosphere);
    float3 baseSkyColor = getBaseSkyColor(atmosphere);

    float3 finalColor = voxelLight * albedoColor.rgb;

    // Fog effect
    float fogFactor = saturate(depthValue * depthValue * DEPTH_SCALE);
    finalColor = lerp(finalColor, baseSkyColor, fogFactor);

    // finalColor = voxelLight;
    // finalColor = pixelLight;
    // finalColor = albedoColor.rgb;
    // finalColor = (abs(motionVector.xy) * p.frame_dim * 0.1).xyx;
    // finalColor = pow(finalColor, 5);
    // finalColor = float3((float2)pixel.xy / p.frame_dim.xy, 0);
    // finalColor = float3((float2)pixel.xy / p.frame_dim.xy, 0);
    // finalColor = float3(ssaoValue, ssaoValue, ssaoValue);

    // Apply firefly removal after all other processing but before final output
    // finalColor = removeFireflies(finalColor, pixel, render_image);

    // post process
    finalColor = tonemapUnreal(finalColor);

    // Debug visualization modes
    if ((bool)p.state_ptr.settings.showNormals)
        finalColor = normalVector;
    if ((bool)p.state_ptr.settings.showHitPos)
        finalColor = worldPos / float(GRID_SIZE);
    if ((bool)p.state_ptr.settings.showDepth)
        finalColor = float3(depthValue / 100.0);
    if ((bool)p.state_ptr.settings.showDepthPrepass)
        finalColor = float3(depthValueHalfRes / 100.0);

    render_image[pixel] = float4(finalColor, 1.0);
}
