#include "shared.inl"
#include "voxel-hashmap.slang"

[[vk::push_constant]]
const ComputePush p;

[numthreads(8, 8, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    // return;
    uint2 pixel = pixel_i.xy;
    if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
        return;

    RWTexture2D<float3> indirect = RWTexture2D<float3>::get(p.gbuffer.indirect);
    RWTexture2D<float4> indirectHistory = RWTexture2D<float4>::get(p.gbuffer.indirectHistory);
    RWTexture2D<int2> voxelIDs = RWTexture2D<int2>::get(p.gbuffer.voxelIDs);
    int2 voxelID = voxelIDs[pixel];

    if (voxelID.x == EMPTY_KEY && voxelID.y == EMPTY_KEY)
    {
        return;
    }

    uint slot = get_hashtable_index(p.past_voxel_hashmap_ptr, voxelID.x, voxelID.y);

    var color = indirect[pixel / 2].xyz;

    if (slot == EMPTY_KEY)
    {
        voxel_hashtable_insert(p.voxel_hashmap_ptr, voxelID.x, voxelID.y, color, 0);
        return;
    }

    float lastHits = p.past_voxel_hashmap_ptr.hitCounts[slot];
    float3 lastColor = (p.past_voxel_hashmap_ptr.indirectLight[slot] / max(lastHits, 1));
    int lastTempFrames = p.past_voxel_hashmap_ptr.temporalFrames[slot];

    float variance = 0.0;
    if (lastTempFrames > 1)
    {
        float3 diff = color - lastColor;
        variance = dot(diff, diff) / (lastTempFrames - 1);
    }

    float lastLuminance = dot(lastColor, float3(0.2126, 0.7152, 0.0722));
    float blendFactor = 1.0 / (lastTempFrames) * (1 - variance);
    blendFactor = clamp(blendFactor, 0.005, 0.9);
    float3 thisFrameIndirect = lastColor * (1 - blendFactor) + color * (blendFactor);
    // indirect[pixel / 2] = thisFrameIndirect;

    voxel_hashtable_insert(p.voxel_hashmap_ptr, voxelID.x, voxelID.y, thisFrameIndirect, lastTempFrames + 1);
}
