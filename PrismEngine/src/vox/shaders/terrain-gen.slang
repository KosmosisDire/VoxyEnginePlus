#include "common.slang"
#include "lygia/generative/random.hlsl"
#include "lygia/generative/snoise.hlsl"

// Terrain generation parameters
static const float noiseScale = 0.07f;
static const uint blockSize = 12;

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
    float scale = noiseScale * (snoise(worldBlockPos * noiseScale * 0.001f) + 1.0f);

    // Height map with 3 levels of noise
    float h = snoise(float2(worldBlockPos.x, worldBlockPos.z) * scale * 0.1f) * 2.0f;
    h += snoise(float2(worldBlockPos.x, worldBlockPos.z) * scale * 0.5f) * 0.5f;
    h += snoise(float2(worldBlockPos.x, worldBlockPos.z) * scale * 0.9f) * 0.25f;
    h = h * 0.5f + 0.5f;
    h *= 1.0f / scale;

    float heightOffset = 90;
    h += heightOffset;

    // Generate holes
    float holes = snoise(worldBlockPos * scale * 0.1);

    // Generate caves
    float caves = snoise(worldBlockPos * scale * 0.5f) * 0.5f + 0.3f;

    // cliffs
    float cliffs = snoise(worldBlockPos * scale * 0.5f);
    cliffs += snoise(worldBlockPos * scale * 0.01f) * 0.5f;

    cliffs = cliffs / (gridPos.y) * GRID_SIZE * CHUNK_SIZE * 4;

    if (cliffs > 0.1)
    {
        h *= 1.1f;
    }

    if (cliffs > 0.8)
    {
        h *= 1.1f;
    }

    return worldBlockPos.y < h && holes > -0.5f && !(caves > -0.05f && caves < 0.05f);
}

// Helper function to set voxel material safely
void setVoxelMaterial(uint brickIndex, uint8_t material)
{
    // Clear the old bits
    p.materialPtrBuffer.data[brickIndex] = material;
}

// Helper function to determine material based on conditions
uint selectMaterial(int3 brickPos, int3 voxelPos)
{
    float noise = snoise(voxelPos * 0.01f + snoise(voxelPos) * 0.03);
    var isSolidAbove = calculateSolid(brickPos * BRICK_SIZE + int3(0, blockSize, 0));
    var isSolidAboveFar = calculateSolid(brickPos * BRICK_SIZE + int3(0, blockSize * 4, 0));

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
    setBit(p.bricksBuffer.data[indices.brickIndex], indices.voxelIndex);
    setBit(p.bricksBuffer, indices.brickIndex / CHUNK_SIZE_CUBE, indices.brickIndex % CHUNK_SIZE_CUBE);
}

// Check if a world position contains a voxel
bool hasWorldVoxel(int3 worldPos)
{
    VoxelIndices indices = getVoxelIndices(worldPos);
    if (!indices.isValid)
        return false;

    BrickBitmask occupancy = p.bricksBuffer.data[indices.brickIndex];
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

    // write to bricks as well
    p.bricksBuffer.data[0].data = {0, 0, 0, 0, 0, 0, 0, 0};
    p.bricksBuffer.data[1].data = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
    

    if (p.passNum == 1)
    {
        var absVoxel = brickCoord * BRICK_SIZE;
        bool isSolid = calculateSolid(absVoxel);
        p.brickPtrBuffer.data[brickIndex] = isSolid ? 1 : 0;
        setVoxelMaterial(brickIndex, selectMaterial(brickCoord, absVoxel));

        // for (uint z = 0; z < BRICK_SIZE; z++)
        // {
        //     for (uint y = 0; y < BRICK_SIZE; y++)
        //     {
        //         for (uint x = 0; x < BRICK_SIZE; x++)
        //         {
        //             int3 localVoxelPos = int3(x, y, z);
        //             int3 absVoxel = brickCoord * BRICK_SIZE + localVoxelPos;
        //             int voxelIndex = voxelLocalIndex(localVoxelPos);

        //             uint globalVoxelIndex = voxelGlobalIndex(brickIndex, voxelIndex);
        //             setVoxelMaterial(globalVoxelIndex, selectMaterial(brickCoord, absVoxel));
        //             // setVoxelMaterial(globalVoxelIndex, 0);
        //         }
        //     }
        // }

        // p.bricksBuffer.data[brickIndex] = brickOccupancy;

        if (isSolid)
        {
            setBit(p.chunksBuffer, chunkIndex, localBrickIndex);
        }
        else
        {
            clearBit(p.chunksBuffer, chunkIndex, localBrickIndex);
        }
    }
}
