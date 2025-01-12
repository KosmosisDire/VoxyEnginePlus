#include "common.slang"

// Set bit in occupancy mask
void setBit(Ptr<Occupancy> buffer, uint index, uint localIndex)
{
    InterlockedOr(buffer.occupancy[index], uint64_t(1) << localIndex);
}

[numthreads(8, 8, 1)]
void main(uint3 absCell: SV_DispatchThreadID)
{
    if (absCell.x >= TREE_SIZE_SQUARE || absCell.y >= TREE_SIZE_SQUARE || absCell.z >= TREE_SIZE_SQUARE)
        return;

    bool isSolid = absCell.y < 5;

    if (isSolid)
    {
        // get local brick coordinates
        int3 brick = (absCell / TREE_SIZE) % TREE_SIZE;

        // get local voxel coordinates
        int3 voxel = absCell % TREE_SIZE;

        // get local index
        int brickIndex = treeLocalIndex(brick);
        int voxelIndex = treeLocalIndex(voxel);

        // set bit in occupancy mask
        setBit(p.chunk_occupancy_ptr, 0, brickIndex);
        setBit(p.brick_occupancy_ptr, brickIndex, voxelIndex);
    }
}
