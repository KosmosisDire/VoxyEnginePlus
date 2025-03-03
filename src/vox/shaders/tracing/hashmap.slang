#include "../voxel-hashmap.slang"
#include "trace.slang"

uint getFaceID(float3 normal, int voxelIndex)
{
    uint faceID = 0;

    // Determine face ID based on normal direction
    // Convert normal to a discrete direction by checking sign of each component
    int xDir = (normal.x > 0.1) ? 1 : ((normal.x < -0.1) ? -1 : 0);
    int yDir = (normal.y > 0.1) ? 1 : ((normal.y < -0.1) ? -1 : 0);
    int zDir = (normal.z > 0.1) ? 1 : ((normal.z < -0.1) ? -1 : 0);

    // Map the 27 possible directions to face IDs (0-26)
    // Using 3x3x3 encoding: (x+1) + (y+1)*3 + (z+1)*9
    faceID = (xDir + 1) + (yDir + 1) * 3 + (zDir + 1) * 9;

    // Center case (0,0,0) is ID 13, skip it for valid faces
    if (faceID == 13)
    {
        // Fallback to the primary axis with the largest component
        float absX = abs(normal.x);
        float absY = abs(normal.y);
        float absZ = abs(normal.z);

        if (absX >= absY && absX >= absZ)
            faceID = normal.x > 0 ? 12 : 10;
        else if (absY >= absX && absY >= absZ)
            faceID = normal.y > 0 ? 16 : 4;
        else
            faceID = normal.z > 0 ? 22 : 4;
    }

    faceID = faceID * BRICK_SIZE_CUBE + voxelIndex;
    return faceID;
}

VoxelHashmapEntry getHashmapEntry(TraceResult trace)
{
    if (!trace.hit)
        return (VoxelHashmapEntry)0;

    int globalBrickIndex = brickGlobalIndex(trace.chunkIndex, trace.brickIndex);
    return get_hashtable_entry(p.voxel_hashmap_ptr, globalBrickIndex, getFaceID(trace.normal, trace.voxelIndex), p.state_ptr.frame);
}

void setHashmapEntry(TraceResult trace, float3 lighting, int historyCount)
{
    if (!trace.hit)
        return;

    int globalBrickIndex = brickGlobalIndex(trace.chunkIndex, trace.brickIndex);
    uint faceID = getFaceID(trace.normal, trace.voxelIndex);
    voxel_hashtable_insert(p.voxel_hashmap_ptr, globalBrickIndex, faceID, lighting, historyCount, p.state_ptr.frame);
}
