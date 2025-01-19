#include "voxel-hashmap.inl"

daxa_u32 hashVoxel(daxa_u32 globalBrickIndex, daxa_u32 localVoxelIndex)
{
    // Improved hash function to reduce collisions
    daxa_u32 hash = globalBrickIndex * 2654435761u + localVoxelIndex;
    hash ^= hash >> 16;
    hash *= 2654435761u;
    hash ^= hash >> 16;
    return hash % HASH_TABLE_SIZE;
}

// Returns the hash index where the entry was stored
daxa_u32 addVoxelToHashmap(
    Ptr<VoxelHashmap> hashmap,
    Ptr<VoxelQueue> queue,
    daxa_u32 globalBrickIndex,
    daxa_u32 localVoxelIndex,
    daxa_f32vec3 indirectLight)
{
    daxa_u32 hash = hashVoxel(globalBrickIndex, localVoxelIndex);
    daxa_u32 originalHash = hash;

    // Linear probing with a maximum number of attempts
    while (true)
    {
        VoxelHashmapEntry entry = hashmap->entries[hash];

        if (entry.visibleCount == 0)
        {
            // Found empty slot, try to claim it atomically
            daxa_u32 original;
            InterlockedCompareExchange(
                hashmap.entries[hash].visibleCount,
                0,       // expected
                1,       // new value
                original // original value
            );

            if (original == 0)
            {
                // Successfully claimed the slot
                hashmap.entries[hash].globalBrickIndex = globalBrickIndex;
                hashmap.entries[hash].localVoxelIndex = localVoxelIndex;
                hashmap.entries[hash].indirectLight = indirectLight;

                // Add to queue atomically
                daxa_u32 queueIndex;
                InterlockedAdd(queue.count, 1, queueIndex);
                if (queueIndex < HASH_TABLE_SIZE)
                {
                    queue.hashmapIndices[queueIndex] = hash;
                }
                return hash;
            }
        }
        else if (entry.globalBrickIndex == globalBrickIndex &&
                 entry.localVoxelIndex == localVoxelIndex)
        {
            // Found existing entry for this voxel
            InterlockedAdd(hashmap.entries[hash].visibleCount, 1);
            return hash;
        }

        // Move to next slot
        hash = (hash + 1) % HASH_TABLE_SIZE;

        // Prevent infinite loop if table is full
        if (hash == originalHash)
        {
            break;
        }
    }

    return MAX_UINT32; // Table is full
}

// Helper function to clear the hashmap and queue
void clearVoxelStructures(
    Ptr<VoxelHashmap> hashmap,
    Ptr<VoxelQueue> queue)
{
    // Only clear the count and visibleCount fields
    queue.count = 0;

    // Clear all entries (can be optimized to clear only used range based on previous queue.count)
    for (daxa_u32 i = 0; i < HASH_TABLE_SIZE; ++i)
    {
        hashmap.entries[i].visibleCount = 0;
    }
}
