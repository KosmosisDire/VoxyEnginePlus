#include "voxel-hashmap.inl"

uint hashVoxel(uint globalBrickIndex, uint localVoxelIndex)
{
    // Improved hash function to reduce collisions
    uint hash = globalBrickIndex * 2654435761u + localVoxelIndex * 967;
    hash = (hash ^ (hash >> 16)) * 2246822519u;
    return hash % HASH_TABLE_SIZE;
}

void voxel_hashtable_insert(VoxelHashmap *hashtable, uint brickIndex, uint voxelIndex, float3 lighting, int temporalFrames, uint frameIndex)
{
    uint slot = hashVoxel(brickIndex, voxelIndex);

    uint tries = 0;
    while (tries < 10)
    {
        uint prev = -1;
        if (frameIndex > hashtable.hitCounts[slot] + 10)
        {
            hashtable.brickIndices[slot] = EMPTY_KEY;
        }

        InterlockedCompareExchange(hashtable.brickIndices[slot], EMPTY_KEY, brickIndex, prev);

        if (prev == EMPTY_KEY || (prev == brickIndex && hashtable.voxelIndices[slot] == voxelIndex))
        {
            hashtable.temporalFrames[slot] = temporalFrames;
            hashtable.voxelIndices[slot] = voxelIndex;
            hashtable.indirectLight[slot] = lighting;
            hashtable.hitCounts[slot] = frameIndex;
            break;
        }

        slot = (slot + 1) & (HASH_TABLE_SIZE - 1);
        tries++;
    }
}

uint get_hashtable_index(VoxelHashmap *hashtable, uint brickIndex, uint voxelIndex)
{
    uint slot = hashVoxel(brickIndex, voxelIndex);

    uint32_t tries = 0;
    while (tries < 10)
    {
        var brickIdx = hashtable.brickIndices[slot];
        var voxelIdx = hashtable.voxelIndices[slot];

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
