#include "voxel-hashmap.inl"

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
    while (tries < 100)
    {
        uint prev = 0;
        InterlockedCompareExchange(hashtable.entries[slot].brickIndex, EMPTY_KEY, brickIndex, prev);
        if (prev == EMPTY_KEY || prev == brickIndex)
        {
            if (prev == EMPTY_KEY)
            {
                InterlockedExchange(hashtable.entries[slot].temporalFrame, temporalFrames);
                InterlockedExchange(hashtable.entries[slot].voxelIndex, voxelIndex);
            }
            else if (hashtable.entries[slot].hitCount > 100)
            {
                break;
            }

            InterlockedAdd(hashtable.entries[slot].hitCount, 1);
            InterlockedAdd(hashtable.entries[slot].indirectLight.x, lighting.x);
            InterlockedAdd(hashtable.entries[slot].indirectLight.y, lighting.y);
            InterlockedAdd(hashtable.entries[slot].indirectLight.z, lighting.z);
            break;
        }
        slot = (slot + 1) & (HASH_TABLE_SIZE - 1);
        tries++;
    }
}

