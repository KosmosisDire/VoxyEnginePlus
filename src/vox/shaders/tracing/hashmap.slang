#include "../voxel-hashmap.slang"
#include "trace.slang"

uint getFaceID(float3 normal, int voxelIndex)
{
    uint faceID = (normal.x != 0 ? 1 : 0) +
                  (normal.y != 0 ? 2 : 0) +
                  (normal.z != 0 ? 4 : 0);
    faceID = faceID * BRICK_SIZE_CUBE + voxelIndex;
    faceID = voxelIndex;
    return faceID;
}

VoxelHashmapEntry getHashmapEntry(TraceResult trace)
{
    if (!trace.hit)
        return (VoxelHashmapEntry)0;

    int globalBrickIndex = brickGlobalIndex(trace.chunkIndex, trace.brickIndex);
    return get_hashtable_entry(p.voxel_hashmap_ptr, globalBrickIndex, getFaceID(trace.normal, trace.brickIndex), p.state_ptr.frame);
}

void setHashmapEntry(TraceResult trace, float3 lighting, int historyCount)
{
    if (!trace.hit)
        return;

    int globalBrickIndex = brickGlobalIndex(trace.chunkIndex, trace.brickIndex);
    uint faceID = getFaceID(trace.normal, trace.brickIndex);
    voxel_hashtable_insert(p.voxel_hashmap_ptr, globalBrickIndex, faceID, lighting, historyCount, p.state_ptr.frame);
}
