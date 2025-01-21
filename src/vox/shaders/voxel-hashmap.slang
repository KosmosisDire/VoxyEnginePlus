#include "voxel-hashmap.inl"

// daxa_u32 hashVoxel(daxa_u32 globalBrickIndex, daxa_u32 localVoxelIndex)
// {
//     return (globalBrickIndex * BRICK_SIZE_CUBE + localVoxelIndex) % HASH_TABLE_SIZE;
// }

uint hashVoxel(uint globalBrickIndex, uint localVoxelIndex)
{
    // Improved hash function to reduce collisions
    uint hash = globalBrickIndex * 2654435761u + localVoxelIndex * 967;
    hash = (hash ^ (hash >> 16)) * 2246822519u;
    return hash % HASH_TABLE_SIZE;
}

void voxel_hashtable_insert(VoxelHashmap *hashtable, uint brickIndex, uint voxelIndex, float3 lighting, int temporalFrames)
{
    uint slot = hashVoxel(brickIndex, voxelIndex);

    uint tries = 0;
    while (tries < 10)
    {
        uint prev = 0;
        InterlockedCompareExchange(hashtable.brickIndices[slot], EMPTY_KEY, brickIndex, prev);
        if (prev == EMPTY_KEY || prev == brickIndex)
        {
            if (prev == EMPTY_KEY)
            {
                InterlockedExchange(hashtable.temporalFrames[slot], temporalFrames);
                InterlockedExchange(hashtable.voxelIndices[slot], voxelIndex);
            }

            InterlockedAdd(hashtable.hitCounts[slot], 1);
            InterlockedAdd(hashtable.indirectLight[slot].x, lighting.x);
            InterlockedAdd(hashtable.indirectLight[slot].y, lighting.y);
            InterlockedAdd(hashtable.indirectLight[slot].z, lighting.z);
            break;
        }
        slot = (slot + 1) & (HASH_TABLE_SIZE - 1);
        tries++;
    }
}

