#include "common.slang"
#include "lygia/generative/snoise.hlsl"

[numthreads(8, 8, 8)]
void main(uint3 brickCoord: SV_DispatchThreadID)
{
    if (p.state_ptr.frame % 300 != 0)
        return;

    int worldBlockSpan = GRID_SIZE * CHUNK_SIZE;

    if (brickCoord.x >= worldBlockSpan || brickCoord.y >= worldBlockSpan || brickCoord.z >= worldBlockSpan)
        return;

    int brickIndex = brickCoord.x + brickCoord.y * worldBlockSpan + brickCoord.z * worldBlockSpan * worldBlockSpan;

    int3 chunkCoord = int3(brickCoord / CHUNK_SIZE);
    int chunkIndex = chunkCoord.x + chunkCoord.y * GRID_SIZE + chunkCoord.z * GRID_SIZE * GRID_SIZE;

    int localBrickIndex = brickCoord.x % CHUNK_SIZE + (brickCoord.y % CHUNK_SIZE) * CHUNK_SIZE + (brickCoord.z % CHUNK_SIZE) * CHUNK_SIZE_SQUARE;

    BrickBitmask brickOccupancy;
    brickOccupancy.part1 = 0;
    brickOccupancy.part2 = 0;
    brickOccupancy.compressed_data_ptr = 0;

    bool hasVoxel = false;
    for (uint i = 0; i < BRICK_SIZE_CUBE; i++)
    {
        int3 localVoxelPos = int3(i % BRICK_SIZE, (i / BRICK_SIZE) % BRICK_SIZE, i / BRICK_SIZE_SQUARE);
        int3 absVoxel = brickCoord * BRICK_SIZE + localVoxelPos;
        bool isSolid = snoise(absVoxel * 0.002f) > 0.5f;

        int voxelIndex = voxelLocalIndex(localVoxelPos);

        if (isSolid)
        {
            // setBit(p.brick_occupancy_ptr, brickIndex, voxelIndex);
            setBit(brickOccupancy, voxelIndex);
            hasVoxel = true;
        }
        else
        {
            // clearBit(p.brick_occupancy_ptr, brickIndex, voxelIndex);
            clearBit(brickOccupancy, voxelIndex);
        }
    }

    p.brick_occupancy_ptr.occupancy[brickIndex] = brickOccupancy;

    if (hasVoxel)
    {
        setBit(p.chunk_occupancy_ptr, chunkIndex, localBrickIndex);
    }
    else
    {
        clearBit(p.chunk_occupancy_ptr, chunkIndex, localBrickIndex);
    }
}
