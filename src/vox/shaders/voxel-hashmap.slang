#include "voxel-hashmap.inl"

uint hashVoxel(uint globalBrickIndex, uint uniqueFaceID)
{
    uint hash = globalBrickIndex * 2654435761u + uniqueFaceID * 967;
    hash = (hash ^ (hash >> 16)) * 2246822519u;
    return hash % HASH_TABLE_SIZE;
}

void voxel_hashtable_insert(VoxelHashmap *hashtable, uint globalBrickIndex, uint uniqueFaceID, float3 lighting, int historyCount, uint frameIndex)
{
    uint slot = hashVoxel(globalBrickIndex, uniqueFaceID);

    uint tries = 0;
    while (tries < 10)
    {
        var entry = hashtable.entries[slot];
        uint prev = -1;
        if (frameIndex > entry.lastContributionFrame + 5)
        {
            hashtable.brickIndices[slot] = EMPTY_KEY;
        }

        InterlockedCompareExchange(hashtable.brickIndices[slot], EMPTY_KEY, globalBrickIndex, prev);

        if (prev == EMPTY_KEY || (prev == globalBrickIndex && entry.uniqueFaceID == uniqueFaceID))
        {
            VoxelHashmapEntry newEntry;
            newEntry.uniqueFaceID = uniqueFaceID;
            newEntry.lighting = lighting;
            newEntry.historyCount = historyCount;
            newEntry.lastContributionFrame = frameIndex;

            hashtable.entries[slot] = newEntry;
            break;
        }

        slot = (slot + 1) & (HASH_TABLE_SIZE - 1);
        tries++;
    }
}

uint get_hashtable_index(VoxelHashmap *hashtable, uint globalBrickIndex, uint uniqueFaceID)
{
    uint slot = hashVoxel(globalBrickIndex, uniqueFaceID);

    uint32_t tries = 0;
    while (tries < 10)
    {
        var entry = hashtable.entries[slot];
        var brickIdx = hashtable.brickIndices[slot];
        var voxelFaceIdx = entry.uniqueFaceID;

        if (brickIdx == globalBrickIndex && voxelFaceIdx == uniqueFaceID)
        {
            return slot;
        }
        if (brickIdx == EMPTY_KEY && voxelFaceIdx == EMPTY_KEY)
        {
            return EMPTY_KEY;
        }

        slot = (slot + 1) & (HASH_TABLE_SIZE - 1);

        tries++;
    }

    return EMPTY_KEY;
}

uint get_hashtable_collision_factor(VoxelHashmap *hashtable, uint globalBrickIndex, uint uniqueFaceID)
{
    uint slot = hashVoxel(globalBrickIndex, uniqueFaceID);

    uint32_t tries = 0;
    while (tries < 10)
    {
        var entry = hashtable.entries[slot];
        var brickIdx = hashtable.brickIndices[slot];
        var voxelFaceIdx = entry.uniqueFaceID;

        if (brickIdx == globalBrickIndex && voxelFaceIdx == uniqueFaceID)
        {
            break;
        }
        if (brickIdx == EMPTY_KEY && voxelFaceIdx == EMPTY_KEY)
        {
            break;
        }

        slot = (slot + 1) & (HASH_TABLE_SIZE - 1);

        tries++;
    }

    return tries;
}
