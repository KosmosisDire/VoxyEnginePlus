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
    uint2 voxelID = voxelIDTex[pixel];
    uint voxelFaceID = voxelFaceIDTex[pixel];
    var entry = get_hashtable_entry(p.voxel_hashmap_ptr, voxelID.x, voxelFaceID, p.state_ptr.frame);

    // Get the current normal and UV coordinates
    float3 normal = normalTex[pixel];
    float2 brickUV = brickUVsTex[pixel];

    // Find the primary axes perpendicular to the normal
    float3 tangent, bitangent;
    if (abs(normal.x) > abs(normal.y))
    {
        tangent = normalize(cross(float3(0.0, 1.0, 0.0), normal));
    }
    else
    {
        tangent = normalize(cross(float3(1.0, 0.0, 0.0), normal));
    }
    bitangent = normalize(cross(normal, tangent));

    // Sample offsets in tangent space
    float offsetAmount = 1.0;
    float3 offsets[4] = {
        tangent * offsetAmount,
        -tangent * offsetAmount,
        bitangent * offsetAmount,
        -bitangent * offsetAmount
    };

    // Map brick UV to relative position on the face (0-1 range)
    // Center point is (0.5, 0.5)
    float2 centerUV = float2(0.5, 0.5);

    // Calculate weights for each direction based on UV position
    // Weights are higher when closer to the edge in that direction
    float weights[4];
    weights[0] = getWeight(brickUV, float2(1.0, 0.5)); // +tangent direction
    weights[1] = getWeight(brickUV, float2(0.0, 0.5)); // -tangent direction
    weights[2] = getWeight(brickUV, float2(0.5, 1.0)); // +bitangent direction
    weights[3] = getWeight(brickUV, float2(0.5, 0.0)); // -bitangent direction

    // Normalize weights to ensure they sum to 1.0 (important for energy conservation)
    float totalWeight = weights[0] + weights[1] + weights[2] + weights[3];
    if (totalWeight > 0.0)
    {
        for (int i = 0; i < 4; i++)
        {
            weights[i] /= totalWeight;
        }
    }
    else
    {
        // If all weights are zero, use center point only
        for (int i = 0; i < 4; i++)
        {
            weights[i] = 0.0;
        }
    }

    // Get center weight based on distance from center
    float centerDistance = length(brickUV - centerUV) * 2.0; // *2 to normalize to 0-1 range
    float centerWeight = 1.0 - min(centerDistance, 1.0);

    // Normalize all weights including center
    float finalTotalWeight = centerWeight + totalWeight;
    centerWeight /= finalTotalWeight;
    for (int i = 0; i < 4; i++)
    {
        weights[i] *= (1.0 - centerWeight);
    }

    // Gather entries from perpendicular directions and interpolate lighting
    float3 interpolatedLighting = float3(0, 0, 1) * 0.1 * centerWeight;

    [unroll]
    for (int i = 0; i < 4; i++)
    {
        if (weights[i] > 0.001) // Skip negligible contributions for performance
        {
            var adjIndex = getAdjacentBrickIndex(voxelID.x, (int3)offsets[i]);
            var adjEntry = get_hashtable_entry(p.voxel_hashmap_ptr, adjIndex, getFaceID(normal, adjIndex), p.state_ptr.frame);

            var lighting = adjEntry.lighting;

            if (adjEntry.historyCount == 0)
            {
                lighting = float3(1, 0, 0) * 0.3;
            }
            else
            {
                lighting = float3(0, 0, 1) * 0.1;
            }

            // Add weighted contribution to lighting
            interpolatedLighting += lighting * weights[i];
        }
    }

    // Rest of the rendering pipeline
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
    color += material.emission;

    color = tonemapUnreal(color * 2.5);
    render_image[pixel] = float4(color, 1.0);
}
