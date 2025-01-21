#include "shared.inl"
#include "voxel-hashmap.slang"

[[vk::push_constant]]
const ComputePush p;

uint get_hashtable_index(uint brickIndex, uint voxelIndex)
{
    uint slot = hashVoxel(brickIndex, voxelIndex);

    uint32_t tries = 0;
    while (tries < 100)
    {
        var brickIdx = p.past_voxel_hashmap_ptr.brickIndices[slot];
        var voxelIdx = p.past_voxel_hashmap_ptr.voxelIndices[slot];

        if (brickIdx == brickIndex && voxelIdx == voxelIndex)
        {
            return slot;
        }
        if (brickIdx == EMPTY_KEY && voxelIdx == EMPTY_KEY)
        {
            return EMPTY_KEY;
        }

        slot = (slot + 1) & (HASH_TABLE_SIZE - 1);

        tries++;
    }

    return EMPTY_KEY;
}

[numthreads(8, 8, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    uint2 pixel = pixel_i.xy;
    if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
        return;

    RWTexture2D<float3> indirect = RWTexture2D<float3>::get(p.gbuffer.indirect);
    RWTexture2D<int2> voxelIDs = RWTexture2D<int2>::get(p.gbuffer.voxelIDs);
    int2 voxelID = voxelIDs[pixel];

    uint slot = get_hashtable_index(voxelID.x, voxelID.y);

    var color = indirect[pixel];
    if (slot == EMPTY_KEY)
    {
        voxel_hashtable_insert(p.voxel_hashmap_ptr, voxelID.x, voxelID.y, color);
        return;
    }

    float3 thisFrameIndirect = (p.past_voxel_hashmap_ptr.indirectLight[slot] / max(p.past_voxel_hashmap_ptr.hitCounts[slot], 1) * (0.9) + color * 0.1);
    // indirect[pixel] = thisFrameIndirect;

    voxel_hashtable_insert(p.voxel_hashmap_ptr, voxelID.x, voxelID.y, thisFrameIndirect);
}
