#include "shared.inl"
#include "voxel-hashmap.slang"

[[vk::push_constant]]
const ComputePush p;

[numthreads(8, 8, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    uint2 pixel = pixel_i.xy;
    if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
        return;

    RWTexture2D<float4> indirect = RWTexture2D<float4>::get(p.gbuffer.indirect);
    RWTexture2D<int2> voxelIDs = RWTexture2D<int2>::get(p.gbuffer.voxelIDs);
    int2 voxelID = voxelIDs[pixel];

    VoxelHashmapEntry lastFrame = getVoxelFromHashmap(p.past_voxel_hashmap_ptr, voxelID.x, voxelID.y);

    float3 lastFrameIndirect = lastFrame.indirectLight / max(lastFrame.hitCount, 1);
    float3 thisFrameIndirect = indirect[pixel].xyz;
    float3 newIndirect = (lastFrameIndirect + thisFrameIndirect) / 2.0;

    addVoxelToHashmap(p.voxel_hashmap_ptr, p.voxel_queue_ptr, voxelIDs[pixel].x, voxelIDs[pixel].y, newIndirect);
}
