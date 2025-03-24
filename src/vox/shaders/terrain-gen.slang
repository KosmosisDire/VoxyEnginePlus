#include "common.slang"
#include "lygia/generative/random.hlsl"
#include "lygia/generative/snoise.hlsl"

// Terrain generation parameters
static const float noiseScale = 0.1f;
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
    h *= 3.0f / scale;
    h += 14.0f;

    // Generate holes
    float holes = snoise(worldBlockPos * scale * 0.1);

    // Generate caves
    float caves = snoise(worldBlockPos * scale * 1.0f) * 0.5f + 0.3f;

    return worldBlockPos.y < h && holes > -0.5f && !(caves > -0.2f && caves < 0.2f);
}

// Helper function to set voxel material safely
void setVoxelMaterial(uint globalVoxelIndex, uint material)
{
    // Clear the old bits
    p.voxel_materials_ptr.materials[globalVoxelIndex / 8] &= ~(0xF << ((globalVoxelIndex % 8) * 4));
    // Set the new bits
    p.voxel_materials_ptr.materials[globalVoxelIndex / 8] |= ((material & 0xF) << ((globalVoxelIndex % 8) * 4));
}

// Helper function to determine material based on conditions
uint selectMaterial(int3 brickPos, int3 voxelPos)
{
    float noise = snoise(voxelPos * 0.01f + snoise(voxelPos) * 0.03);
    var isSolidAbove = calculateSolid(brickPos * blockSize + int3(0, blockSize, 0));
    var isSolidAboveFar = calculateSolid(brickPos * blockSize + int3(0, blockSize * 4, 0));

    if (!isSolidAbove)
    {
        return 1;
    }
    else if (!isSolidAboveFar)
    {
        return 5;
    }
    else if (noise < -0.7)
    {
        return 6;
    }
    else
    {
        return 2;
    }
}

// function to set whole brick to full with a material
void fillBrick(uint brickIndex, uint material)
{
    if (brickIndex >= TOTAL_BRICKS)
        return;

    BrickBitmask brickOccupancy;
    brickOccupancy.data = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};

    for (uint i = 0; i < BRICK_SIZE * BRICK_SIZE * BRICK_SIZE; i++)
    {
        setVoxelMaterial(voxelGlobalIndex(brickIndex, i), material);
    }

    p.brick_occupancy_ptr.occupancy[brickIndex] = brickOccupancy;
    p.chunk_occupancy_ptr.occupancy[brickIndex / CHUNK_SIZE_CUBE] = 0xFFFFFFFF;
}

void clearBrick(uint brickIndex)
{
    BrickBitmask brickOccupancy;
    brickOccupancy.data = {0, 0, 0, 0, 0, 0, 0, 0};

    for (uint i = 0; i < BRICK_SIZE * BRICK_SIZE * BRICK_SIZE; i++)
    {
        clearBit(brickOccupancy, i);
    }

    p.brick_occupancy_ptr.occupancy[brickIndex] = brickOccupancy;
    p.chunk_occupancy_ptr.occupancy[brickIndex / CHUNK_SIZE_CUBE] = 0;
}


// Structure to hold brick and voxel indices
struct VoxelIndices
{
    int brickIndex;
    int voxelIndex;
    bool isValid;
};

// Get brick and voxel indices from world position
VoxelIndices getVoxelIndices(int3 worldPos)
{
    VoxelIndices result;

    // Calculate brick position
    int3 brickPos = worldPos / BRICK_SIZE;
    int3 localPos = worldPos % BRICK_SIZE;

    // Handle negative positions correctly
    if (any(localPos < 0))
    {
        brickPos -= 1;
        localPos = (localPos + BRICK_SIZE) % BRICK_SIZE;
    }

    // Check world boundaries
    int worldBlockSpan = GRID_SIZE * CHUNK_SIZE;
    if (any(brickPos < 0) || any(brickPos >= worldBlockSpan))
    {
        result.isValid = false;
        return result;
    }

    // Calculate indices
    int3 chunkPos = brickPos / CHUNK_SIZE;
    int3 localBrickPos = brickPos % CHUNK_SIZE;

    int chunkIndex = chunkIndex(chunkPos);
    int localBrickIndex = brickLocalIndex(localBrickPos);

    result.brickIndex = brickGlobalIndex(chunkIndex, localBrickIndex);
    result.voxelIndex = voxelLocalIndex(localPos);
    result.isValid = true;

    return result;
}

// Set a voxel at any world position
void setWorldVoxel(int3 worldPos, uint material)
{
    VoxelIndices indices = getVoxelIndices(worldPos);
    if (!indices.isValid)
        return;

    // Set material
    uint globalVoxelIndex = voxelGlobalIndex(indices.brickIndex, indices.voxelIndex);
    setVoxelMaterial(globalVoxelIndex, material);

    // Update occupancy
    setBit(p.brick_occupancy_ptr.occupancy[indices.brickIndex], indices.voxelIndex);
    setBit(p.chunk_occupancy_ptr, indices.brickIndex / CHUNK_SIZE_CUBE, indices.brickIndex % CHUNK_SIZE_CUBE);
}

// Check if a world position contains a voxel
bool hasWorldVoxel(int3 worldPos)
{
    VoxelIndices indices = getVoxelIndices(worldPos);
    if (!indices.isValid)
        return false;

    BrickBitmask occupancy = p.brick_occupancy_ptr.occupancy[indices.brickIndex];
    return getBit(occupancy, indices.voxelIndex);
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

    if (p.pass == 1)
    {
        BrickBitmask brickOccupancy;
        brickOccupancy.data = {0, 0, 0, 0, 0, 0, 0, 0};

        bool hasVoxel = false;
        for (uint z = 0; z < BRICK_SIZE; z++)
        {
            for (uint y = 0; y < BRICK_SIZE; y++)
            {
                for (uint x = 0; x < BRICK_SIZE; x++)
                {
                    int3 localVoxelPos = int3(x, y, z);
                    int3 absVoxel = brickCoord * BRICK_SIZE + localVoxelPos;
                    int voxelIndex = voxelLocalIndex(localVoxelPos);

                    bool isSolid = calculateSolid(absVoxel);

                    if (isSolid)
                    {
                        uint globalVoxelIndex = voxelGlobalIndex(brickIndex, voxelIndex);
                        setVoxelMaterial(globalVoxelIndex, selectMaterial(brickCoord, absVoxel));
                        setBit(brickOccupancy, voxelIndex);

                        hasVoxel = true;
                    }
                    else
                    {
                        if (absVoxel.y <= 400) // water
                        {
                            uint globalVoxelIndex = voxelGlobalIndex(brickIndex, voxelIndex);
                            setVoxelMaterial(globalVoxelIndex, 9);
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
}
