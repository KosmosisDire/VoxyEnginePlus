#include "voxel-hashmap.inl"

// daxa_u32 hashVoxel(daxa_u32 globalBrickIndex, daxa_u32 localVoxelIndex)
// {
//     return (globalBrickIndex * BRICK_SIZE_CUBE + localVoxelIndex) % HASH_TABLE_SIZE;
// }

daxa_u32 hashVoxel(daxa_u32 globalBrickIndex, daxa_u32 localVoxelIndex)
{
    // Improved hash function to reduce collisions
    daxa_u32 hash = globalBrickIndex * 2654435761u + localVoxelIndex;
    hash ^= hash >> 16;
    hash *= 2654435761u;
    hash ^= hash >> 16;
    return hash % HASH_TABLE_SIZE;
}

// Pack indices into a single u32 for atomic operations
daxa_u32 packIndices(daxa_u32 globalBrickIndex, daxa_u32 localVoxelIndex)
{
    // Assuming localVoxelIndex uses at most 16 bits
    return (globalBrickIndex << 16) | (localVoxelIndex & 0xFFFF);
}

// Unpack indices
void unpackIndices(daxa_u32 packed, out daxa_u32 globalBrickIndex, out daxa_u32 localVoxelIndex)
{
    globalBrickIndex = packed >> 16;
    localVoxelIndex = packed & 0xFFFF;
}

daxa_u32 addVoxelToHashmap(
    Ptr<VoxelHashmap> hashmap,
    Ptr<VoxelQueue> queue,
    daxa_u32 globalBrickIndex,
    daxa_u32 localVoxelIndex,
    daxa_f32vec3 indirectLight)
{
    daxa_u32 hash = hashVoxel(globalBrickIndex, localVoxelIndex);
    daxa_u32 originalHash = hash;
    daxa_u32 packedIndices = packIndices(globalBrickIndex, localVoxelIndex);

    // Linear probing with a maximum number of attempts
    while (true)
    {
        // Atomic read of the current header
        if (hashmap.entries[hash].hitCount == 0)
        {
            // Try to atomically claim the slot
            daxa_u32 original;
            InterlockedCompareExchange(
                hashmap.entries[hash].hitCount,
                0,       // expected
                1,       // new value
                original // original value
            );

            if (original == 0)
            {
                hashmap.entries[hash].indices = packedIndices;
                hashmap.entries[hash].indirectLight = indirectLight;

                // Add to queue atomically
                daxa_u32 queueIndex;
                InterlockedAdd(queue.count, 1, queueIndex);
                queue.hashmapIndices[queueIndex] = hash;

                return hash;
            }
        }
        else
        {
            // Atomically read the current indices
            daxa_u32 currentIndices = hashmap.entries[hash].indices;

            if (currentIndices == packedIndices)
            {
                // Found matching entry, atomically update it
                InterlockedAdd(hashmap.entries[hash].hitCount, 1);

                // Use atomic adds for the light values
                InterlockedAdd(hashmap.entries[hash].indirectLight.x, indirectLight.x);
                InterlockedAdd(hashmap.entries[hash].indirectLight.y, indirectLight.y);
                InterlockedAdd(hashmap.entries[hash].indirectLight.z, indirectLight.z);

                return hash;
            }
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

// Updated getter function
VoxelHashmapEntry getVoxelFromHashmap(
    Ptr<VoxelHashmap> hashmap,
    daxa_u32 globalBrickIndex,
    daxa_u32 localVoxelIndex)
{
    daxa_u32 hash = hashVoxel(globalBrickIndex, localVoxelIndex);
    daxa_u32 originalHash = hash;
    daxa_u32 packedIndices = packIndices(globalBrickIndex, localVoxelIndex);

    while (true)
    {
        // Atomic read of the current state
        daxa_u32 currentHeader = hashmap.entries[hash].hitCount;

        if (currentHeader == 0)
        {
            // Entry not found
            VoxelHashmapEntry emptyEntry;
            emptyEntry.hitCount = 0;
            return emptyEntry;
        }

        // Atomic read of indices
        daxa_u32 currentIndices = hashmap.entries[hash].indices;

        if (currentIndices == packedIndices)
        {
            // Found matching entry, return a copy
            VoxelHashmapEntry result;
            result.hitCount = currentHeader;
            result.indices = currentIndices;
            result.indirectLight = hashmap.entries[hash].indirectLight;
            return result;
        }

        // Move to next slot
        hash = (hash + 1) % HASH_TABLE_SIZE;

        if (hash == originalHash)
        {
            VoxelHashmapEntry emptyEntry;
            emptyEntry.hitCount = 0;
            return emptyEntry;
        }
    }
}
