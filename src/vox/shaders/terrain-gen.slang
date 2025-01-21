#include "common.slang"
#include "lygia/generative/snoise.hlsl"

// Terrain generation parameters
static const float noiseScale = 0.3f;
static const uint blockSize = 6;

// Convert grid coordinates to world position for noise sampling
float3 gridToWorldPos(int3 gridPos)
{
    return float3(gridPos) * WORLD_VOXEL_SIZE;
}

// Get world position aligned to blocks for consistent terrain
float3 getBlockAlignedPos(int3 gridPos)
{
    float3 worldPos = gridToWorldPos(gridPos);
    return float3(
        floor(worldPos.x / (WORLD_VOXEL_SIZE * blockSize)) * (WORLD_VOXEL_SIZE * blockSize),
        floor(worldPos.y / (WORLD_VOXEL_SIZE * blockSize)) * (WORLD_VOXEL_SIZE * blockSize),
        floor(worldPos.z / (WORLD_VOXEL_SIZE * blockSize)) * (WORLD_VOXEL_SIZE * blockSize));
}

// Calculate if a position should be solid based on height map and caves
bool calculateSolid(int3 gridPos)
{
    float3 worldBlockPos = getBlockAlignedPos(gridPos);
    float scale = noiseScale * (snoise(worldBlockPos * noiseScale * 0.1f) + 1.0f);

    // Height map with 3 levels of noise
    float h = snoise(float2(worldBlockPos.x, worldBlockPos.z) * scale * 0.1f) * 2.0f;
    h += snoise(float2(worldBlockPos.x, worldBlockPos.z) * scale * 0.5f) * 0.5f;
    h += snoise(float2(worldBlockPos.x, worldBlockPos.z) * scale * 0.9f) * 0.25f;
    h = h * 0.5f + 0.5f;
    h *= 2.0f / noiseScale;
    h += 39.0f;

    // Generate holes
    float holes = snoise(worldBlockPos * scale * 0.1);

    // Generate caves
    float caves = snoise(worldBlockPos * scale * 1.0f) * 0.5f + 0.3f;

    return worldBlockPos.y < h && holes > -0.6f && !(caves > -0.2f && caves < 0.2f);
}

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

    // Iterate through each voxel in the brick
    for (uint z = 0; z < BRICK_SIZE; z++)
    {
        for (uint y = 0; y < BRICK_SIZE; y++)
        {
            for (uint x = 0; x < BRICK_SIZE; x++)
            {
                int3 localVoxelPos = int3(x, y, z);
                int3 absVoxel = brickCoord * BRICK_SIZE + localVoxelPos;

                bool isSolid = calculateSolid(absVoxel);
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
