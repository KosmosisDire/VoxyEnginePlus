#include "common.slang"
#include "lygia/generative/snoise.hlsl"

[numthreads(4, 4, 4)]
void main(uint3 brickCoord: SV_DispatchThreadID, uint3 localBrickCoord: SV_GroupThreadID)
{
    int worldBlockSpan = GRID_SIZE * CHUNK_SIZE;

    if (brickCoord.x >= worldBlockSpan || brickCoord.y >= worldBlockSpan || brickCoord.z >= worldBlockSpan)
        return;

    int3 chunkCoord = int3(brickCoord / CHUNK_SIZE);
    int chunkIndex = chunkIndex(chunkCoord);

    int localBrickIndex = brickLocalIndex(localBrickCoord);
    int brickIndex = brickGlobalIndex(chunkIndex, localBrickIndex);

    BrickBitmask brickOccupancy;
    brickOccupancy.part1 = 0;
    brickOccupancy.part2 = 0;
    brickOccupancy.compressed_data_ptr = 0;

    bool hasVoxel = false;

    // Iterate through each voxel in the brick using 3D nested loops
    for (uint z = 0; z < BRICK_SIZE; z++)
    {
        for (uint y = 0; y < BRICK_SIZE; y++)
        {
            for (uint x = 0; x < BRICK_SIZE; x++)
            {
                int3 localVoxelPos = int3(x, y, z);
                int3 absVoxel = brickCoord * BRICK_SIZE + localVoxelPos;

                float3 noiseCoord = absVoxel * 0.002f;
                float noiseValue = smoothstep(0.0f, 1.0f, snoise(noiseCoord));
                bool isSolid = noiseValue > 0.01f;

                int voxelIndex = voxelLocalIndex(localVoxelPos);

                if (isSolid)
                {
                    setBit(brickOccupancy, voxelIndex);
                    hasVoxel = true;
                }
                else
                {
                    clearBit(brickOccupancy, voxelIndex);
                }
            }
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
