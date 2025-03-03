#include "atmosphere.slang"
#include "lygia/color/tonemap/unreal.hlsl"
#include "tracing/hashmap.slang"

// Function to get the weight for interpolation based on distance
float getWeight(float2 uv, float2 targetPos)
{
    float2 diff = abs(uv - targetPos);
    float dist = max(diff.x, diff.y);
    return 1.0 - min(dist, 1.0);
}

[numthreads(8, 8, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    uint2 pixel = pixel_i.xy;
    if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
        return;

    RWTexture2D<float4> render_image = RWTexture2D<float4>::get(p.final_image);
    RWTexture2D<float3> albedoTex = RWTexture2D<float3>::get(p.gbuffer.color);
    RWTexture2D<float3> indirectTex = RWTexture2D<float3>::get(p.gbuffer.indirectDenoised);
    RWTexture2D<float3> indirectPerVoxPassTex = RWTexture2D<float3>::get(p.gbuffer.indirectPerVoxelPass2);
    RWTexture2D<uint2> voxelIDTex = RWTexture2D<uint2>::get(p.gbuffer.voxelIDs);
    RWTexture2D<uint> voxelFaceIDTex = RWTexture2D<uint>::get(p.gbuffer.voxelFaceIDs);
    RWTexture2D<uint> materialIDTex = RWTexture2D<uint>::get(p.gbuffer.materialIDs);
    RWTexture2D<float2> voxelUVsTex = RWTexture2D<float2>::get(p.gbuffer.voxelUVs);
    RWTexture2D<float2> brickUVsTex = RWTexture2D<float2>::get(p.gbuffer.brickUVs);
    RWTexture2D<float> shadowTex = RWTexture2D<float>::get(p.gbuffer.shadow);
    RWTexture2D<float3> normalTex = RWTexture2D<float3>::get(p.gbuffer.normal);

    // Get current entry
    float3 indirect = indirectTex[pixel / 2];
    uint2 voxelID = voxelIDTex[pixel];
    uint voxelFaceID = voxelFaceIDTex[pixel];
    var entry = get_hashtable_entry(p.voxel_hashmap_ptr, voxelID.x, voxelFaceID, p.state_ptr.frame);

    // Get the current normal and UV coordinates
    float3 normal = normalTex[pixel];
    float2 brickUV = brickUVsTex[pixel];

    float3 albedo = albedoTex[pixel];
    float shadow = shadowTex[pixel];
    uint materialID = materialIDTex[pixel];
    materialID = clamp(materialID, 0, TOTAL_MATERIALS - 1);
    Material material = p.materials_ptr.materials[materialID];
    float ldot = max(dot(normal, p.state_ptr.sunDir), 0.0);

    float3 color = float3(0);
    color = albedo;
    color += shadow * ldot * getSunColor(initAtmosphere(normal, p.state_ptr.sunDir)) * albedo * 1;
    color *= shadow / 2.0 + 1.0;

    // Use interpolated lighting instead of single entry
    // color *= interpolatedLighting;
    color *= entry.lighting;
    // color *= indirect;
    // color += material.emission;

    color = tonemapUnreal(color);

    if (p.state_ptr.settings.showNormals)
    {
        color = normal * 0.5 + 0.5;
    }

    render_image[pixel] = float4(float3(color), 1.0);
}
