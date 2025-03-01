#include "voxel-hashmap.inl"

uint hashVoxel(uint globalBrickIndex, uint uniqueFaceID)
{
    uint hash = globalBrickIndex * 2654435761u + uniqueFaceID * 967;
    hash = (hash ^ (hash >> 16)) * 2246822519u;
    hash = (hash ^ (hash >> 13)) * 3266489917u;
    hash = hash ^ (hash >> 16);
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
        if (tries > 2 && frameIndex > entry.lastContributionFrame + 5)
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

VoxelHashmapEntry get_hashtable_entry(VoxelHashmap *hashtable, uint globalBrickIndex, uint uniqueFaceID, uint frame)
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
            // make sure frame is updated since it was accessed
            hashtable.entries[slot].lastContributionFrame = frame;
            return entry;
        }
        if (brickIdx == EMPTY_KEY && voxelFaceIdx == EMPTY_KEY)
        {
            return (VoxelHashmapEntry)0;
        }

        slot = (slot + 1) & (HASH_TABLE_SIZE - 1);

        tries++;
    }

    return (VoxelHashmapEntry)0;
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

