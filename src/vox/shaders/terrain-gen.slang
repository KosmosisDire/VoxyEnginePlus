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
    brickOccupancy.part1 = 0xFFFFFFFF;
    brickOccupancy.part2 = 0xFFFFFFFF;
    brickOccupancy.compressed_data_ptr = 0xFFFFFFFF;

    for (uint i = 0; i < BRICK_SIZE * BRICK_SIZE * BRICK_SIZE; i++)
    {
        setVoxelMaterial(voxelGlobalIndex(brickIndex, i), material);
    }

    p.brick_occupancy_ptr.occupancy[brickIndex] = brickOccupancy;
    p.chunk_occupancy_ptr.occupancy[brickIndex / CHUNK_SIZE_CUBE] = 0xFFFFFFFF;
}

void fillBrickCylinder(uint brickIndex, uint material, float3 center, float radius, int height)
{
    if (brickIndex >= TOTAL_BRICKS)
        return;

    BrickBitmask brickOccupancy;
    brickOccupancy.part1 = 0;
    brickOccupancy.part2 = 0;
    brickOccupancy.compressed_data_ptr = 0;

    for (uint z = 0; z < BRICK_SIZE; z++)
    {
        for (uint y = 0; y < BRICK_SIZE; y++)
        {
            for (uint x = 0; x < BRICK_SIZE; x++)
            {
                int3 localVoxelPos = int3(x, y, z);
                int3 absVoxel = brickIndex * BRICK_SIZE + localVoxelPos;

                // Calculate distance from center
                float3 distance = localVoxelPos - (center - 0.5f);
                distance.y = abs(distance.y);

                // Check if inside cylinder
                if (distance.x * distance.x + distance.z * distance.z < radius * radius &&
                    distance.y < height)
                {
                    int voxelIndex = voxelLocalIndex(localVoxelPos);
                    setVoxelMaterial(voxelGlobalIndex(brickIndex, voxelIndex), material);
                    setBit(brickOccupancy, voxelIndex);
                }
            }
        }
    }

    p.brick_occupancy_ptr.occupancy[brickIndex] = brickOccupancy;
    p.chunk_occupancy_ptr.occupancy[brickIndex / CHUNK_SIZE_CUBE] = 0xFFFFFFFF;
}

void fillBrickRandom(uint brickIndex, uint material, float density)
{
    if (brickIndex >= TOTAL_BRICKS)
        return;

    BrickBitmask brickOccupancy;
    brickOccupancy.part1 = 0;
    brickOccupancy.part2 = 0;
    brickOccupancy.compressed_data_ptr = 0;

    for (uint z = 0; z < BRICK_SIZE; z++)
    {
        for (uint y = 0; y < BRICK_SIZE; y++)
        {
            for (uint x = 0; x < BRICK_SIZE; x++)
            {
                int3 localVoxelPos = int3(x, y, z);
                int3 absVoxel = brickIndex * BRICK_SIZE + localVoxelPos;

                // Use noise to determine if voxel should be filled
                float noise = random(localVoxelPos + (brickIndex / 18924.8327));
                if (noise < density)
                {
                    int voxelIndex = voxelLocalIndex(localVoxelPos);
                    setVoxelMaterial(voxelGlobalIndex(brickIndex, voxelIndex), material);
                    setBit(brickOccupancy, voxelIndex);
                }
            }
        }
    }

    p.brick_occupancy_ptr.occupancy[brickIndex] = brickOccupancy;
    p.chunk_occupancy_ptr.occupancy[brickIndex / CHUNK_SIZE_CUBE] = 0xFFFFFFFF;
}

void clearBrick(uint brickIndex)
{
    BrickBitmask brickOccupancy;
    brickOccupancy.part1 = 0;
    brickOccupancy.part2 = 0;
    brickOccupancy.compressed_data_ptr = 0;

    for (uint i = 0; i < BRICK_SIZE * BRICK_SIZE * BRICK_SIZE; i++)
    {
        clearBit(brickOccupancy, i);
    }

    p.brick_occupancy_ptr.occupancy[brickIndex] = brickOccupancy;
    p.chunk_occupancy_ptr.occupancy[brickIndex / CHUNK_SIZE_CUBE] = 0;
}

void generateTree(int3 baseBrickCoord)
{
    // Get world boundaries
    int worldBlockSpan = GRID_SIZE * CHUNK_SIZE;

    // Use noise to determine tree height (8-20 bricks tall)
    float heightNoise = random(float3(baseBrickCoord) / 128.28);
    int treeHeight = int(8 + heightNoise * 40);

    // Calculate maximum possible height based on world bounds
    int maxPossibleHeight = (worldBlockSpan - baseBrickCoord.y);
    treeHeight = min(treeHeight, maxPossibleHeight);

    if (treeHeight < 8)
        return; // Not enough space for a proper tree

    // Determine if tree should have thick trunk
    bool hasThickTrunk = treeHeight > 35;

    for (int y = -2; y < treeHeight; y++)
    {
        // For thick trunks, create a 2x2 pattern
        if (hasThickTrunk)
        {
            [unroll]
            for (int dx = 0; dx <= 1; dx++)
            {
                [unroll]
                for (int dz = 0; dz <= 1; dz++)
                {
                    int3 trunkPos = baseBrickCoord + int3(dx, y, dz);
                    if (trunkPos.y >= worldBlockSpan)
                        continue;

                    // Get brick index for this position
                    int chunkIndex = chunkIndex(trunkPos / CHUNK_SIZE);
                    int localBrickIndex = brickLocalIndex(trunkPos % CHUNK_SIZE);
                    int brickIndex = brickGlobalIndex(chunkIndex, localBrickIndex);

                    // Fill with log material (3)
                    fillBrick(brickIndex, 3);
                }
            }
        }
        else
        {
            // Original single trunk code
            int3 trunkPos = baseBrickCoord + int3(0, y, 0);
            if (trunkPos.y >= worldBlockSpan)
                break;

            int chunkIndex = chunkIndex(trunkPos / CHUNK_SIZE);
            int localBrickIndex = brickLocalIndex(trunkPos % CHUNK_SIZE);
            int brickIndex = brickGlobalIndex(chunkIndex, localBrickIndex);

            fillBrick(brickIndex, 3);
        }
    }

    // Generate leaves in a cone shape with alternating pattern
    float baseRadius = treeHeight * 0.25f; // Maximum radius at the bottom
    baseRadius = clamp(baseRadius, 2.0f, 6.0f);

    if (hasThickTrunk)
        baseRadius *= 1.5f;

    for (int y = 0; y < treeHeight; y++)
    {
        // Calculate the radius for this layer
        float heightPercent = float(y) / float(treeHeight);
        float baseLayerRadius = baseRadius * (1.0f - heightPercent); // Cone shape

        // Add alternating pattern
        float alternatingFactor = sin(heightPercent * 3.14159f * 11) * 0.3f + 0.3f;
        float layerRadius = baseLayerRadius * (1.0f + alternatingFactor);
        int currentRadius = int(layerRadius + 0.5f);

        for (int x = -currentRadius; x <= currentRadius; x++)
        {
            for (int z = -currentRadius; z <= currentRadius; z++)
            {
                // Create circular pattern using distance check
                float distance = sqrt(x * x + z * z);
                if (distance > layerRadius)
                    continue;

                if (y <= 2 && distance < currentRadius - (y + 1))
                    continue;

                // Skip trunk positions for thick trunk
                if (y < treeHeight - 8)
                {
                    if (hasThickTrunk)
                    {
                        if ((x == 0 || x == 1) && (z == 0 || z == 1))
                            continue;
                    }
                    else
                    {
                        if (x == 0 && z == 0)
                            continue;
                    }
                }

                int3 leafPos = baseBrickCoord + int3(x, y + 8, z);

                // Check world boundaries
                if (leafPos.x < 0 || leafPos.x >= worldBlockSpan ||
                    leafPos.y < 0 || leafPos.y >= worldBlockSpan ||
                    leafPos.z < 0 || leafPos.z >= worldBlockSpan)
                    continue;

                // Add some noise to leaf placement
                float leafNoise = random(leafPos);
                if (leafNoise < 0.05)
                    continue;

                // Get brick index for leaves
                int chunkIndex = chunkIndex(leafPos / CHUNK_SIZE);
                int localBrickIndex = brickLocalIndex(leafPos % CHUNK_SIZE);
                int brickIndex = brickGlobalIndex(chunkIndex, localBrickIndex);

                // Fill with leaf material (4)
                fillBrickRandom(brickIndex, 4, min(heightPercent + 0.3, 0.9));
            }
        }
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

// Generate a flower at any world position
void generateFlower(int3 baseWorldPos, uint material)
{
    // Generate stem (2-3 voxels tall)
    float heightNoise = random(baseWorldPos * 1.7324f);
    int stemHeight = int(heightNoise * 4);

    // Create stem
    for (int y = 0; y < stemHeight; y++)
    {
        int3 stemPos = baseWorldPos + int3(0, y, 0);
        setWorldVoxel(stemPos, 4); // Stem material
    }

    // Create flower head
    int3 headPos = baseWorldPos + int3(0, stemHeight, 0);

    // Center
    setWorldVoxel(headPos, material);

    // Petals
    int3 offsets[4] = {
        int3(1, 0, 0),
        int3(-1, 0, 0),
        int3(0, 0, 1),
        int3(0, 0, -1)
    };

    for (int i = 0; i < 4; i++)
    {
        setWorldVoxel(headPos + offsets[i], material);
    }
}

// Add bush generation function
void generateBush(int3 baseBrickCoord)
{
    int worldBlockSpan = GRID_SIZE * CHUNK_SIZE;

    // Use noise for bush size (2-4 bricks)
    float sizeNoise = random(float3(baseBrickCoord) / 64.28);
    int bushSize = int(2 + sizeNoise * 3);

    // Calculate maximum possible height based on world bounds
    int maxPossibleHeight = (worldBlockSpan - baseBrickCoord.y);
    bushSize = min(bushSize, maxPossibleHeight);

    if (bushSize < 2)
        return;

    float baseRadius = bushSize * 0.8f;

    // Generate roughly spherical bush
    for (int y = 0; y < bushSize; y++)
    {
        float heightPercent = float(y) / float(bushSize);
        float layerRadius = baseRadius * (1.0f - pow(heightPercent - 0.5f, 2) * 2);
        int currentRadius = int(layerRadius + 0.5f);

        for (int x = -currentRadius; x <= currentRadius; x++)
        {
            for (int z = -currentRadius; z <= currentRadius; z++)
            {
                float distance = sqrt(x * x + z * z);
                if (distance > layerRadius)
                    continue;

                int3 leafPos = baseBrickCoord + int3(x, y, z);

                // Check world boundaries
                if (leafPos.x < 0 || leafPos.x >= worldBlockSpan ||
                    leafPos.y < 0 || leafPos.y >= worldBlockSpan ||
                    leafPos.z < 0 || leafPos.z >= worldBlockSpan)
                    continue;

                // Add some noise to leaf placement
                float leafNoise = random(leafPos);
                if (leafNoise < 0.1)
                    continue;

                // Get brick index for leaves
                int chunkIndex = chunkIndex(leafPos / CHUNK_SIZE);
                int localBrickIndex = brickLocalIndex(leafPos % CHUNK_SIZE);
                int brickIndex = brickGlobalIndex(chunkIndex, localBrickIndex);

                // Fill with bush material (7 - new material for bushes)
                fillBrickRandom(brickIndex, 4, 0.7);
            }
        }
    }
}

// Vine generation parameters
static const int maxVineLength = 2000;
static const float branchChance = 0.15;
static const float flowerChance = 0.1;
static const float hangingChance = 0.95;

// Direction vectors for checking adjacent blocks
static const int3 directions[6] = {
    int3(0, 1, 0),  // up
    int3(0, -1, 0), // down
    int3(1, 0, 0),  // right
    int3(-1, 0, 0), // left
    int3(0, 0, 1),  // forward
    int3(0, 0, -1)  // back
};

// Helper to check if a position has a valid non-vine block to attach to
bool hasValidAttachment(int3 pos)
{
    for (int i = 0; i < 6; i++)
    {
        int3 adjPos = pos + directions[i];
        VoxelIndices indices = getVoxelIndices(adjPos);
        if (!indices.isValid)
            continue;

        uint globalVoxelIndex = voxelGlobalIndex(indices.brickIndex, indices.voxelIndex);
        uint material = (p.voxel_materials_ptr.materials[globalVoxelIndex / 8] >> ((globalVoxelIndex % 8) * 4)) & 0xF;

        if (hasWorldVoxel(adjPos) && material != 4)
        {
            return true;
        }
    }
    return false;
}

// Get next position for hanging vine with sine wave motion
int3 getHangingVinePosition(int3 startPos, int3 currentPos, int dropDistance)
{
    // Pick a consistent axis for the vine to wave along based on start position
    float axisNoise = random(startPos * 1.234);
    bool useXAxis = axisNoise > 0.5;

    // Get noise-based frequency variation
    float freqNoise = (snoise(float3(startPos) * 0.05) + 1.0) * 0.5;
    float frequency = 0.2 + freqNoise * 0.3; // Range of 0.2 to 0.5

    // Calculate offset from center based on drop distance
    float wave = sin(dropDistance * frequency) * 1.2;
    int offset = int(round(wave));

    // Apply offset to chosen axis while keeping centered around start position
    int3 nextPos;
    if (useXAxis)
    {
        nextPos = int3(startPos.x + offset, currentPos.y - 1, startPos.z);
    }
    else
    {
        nextPos = int3(startPos.x, currentPos.y - 1, startPos.z + offset);
    }

    return nextPos;
}

void generateVine(int3 startPos)
{
    if (!hasValidAttachment(startPos))
        return;

    // Queue for branches
    int3 branchPoints[8];
    int branchCount = 0;

    // Current growth point
    int3 currentPos = startPos;

    // Track if we're in a hanging section
    bool isHanging = false;
    int hangingStartHeight = 0;
    int3 hangingStartPos = int3(0, 0, 0);

    // Main vine
    for (int i = 0; i < maxVineLength; i++)
    {
        // Place vine segment
        setWorldVoxel(currentPos, 4);

        // Maybe place flower
        if (random(currentPos * 1.234) < flowerChance)
        {
            for (int j = 0; j < 6; j++)
            {
                int3 flowerPos = currentPos + directions[j];
                if (!hasWorldVoxel(flowerPos))
                {
                    setWorldVoxel(flowerPos, 6);
                    break;
                }
            }
        }

        // Maybe create branch point (only if not hanging)
        if (!isHanging && random(currentPos * 2.345) < branchChance && branchCount < 8)
        {
            branchPoints[branchCount] = currentPos;
            branchCount++;
        }

        int3 nextPos;
        if (!isHanging)
        {
            // Try to find a valid surface position
            bool foundNext = false;
            float bestScore = -1;

            for (int x = -1; x <= 1; x++)
            {
                for (int y = -1; y <= 1; y++)
                {
                    for (int z = -1; z <= 1; z++)
                    {
                        if (x == 0 && y == 0 && z == 0)
                            continue;

                        int3 testPos = currentPos + int3(x, y, z);
                        if (hasWorldVoxel(testPos))
                            continue;

                        if (hasValidAttachment(testPos))
                        {
                            float score = random(testPos * 3.456);
                            if (score > bestScore)
                            {
                                bestScore = score;
                                nextPos = testPos;
                                foundNext = true;
                            }
                        }
                    }
                }
            }

            // If no valid surface position, consider starting a hanging section
            if (!foundNext && random(currentPos * 1.123) < hangingChance)
            {
                isHanging = true;
                hangingStartPos = currentPos;
                hangingStartHeight = currentPos.y;
                nextPos = getHangingVinePosition(currentPos, currentPos, 0);
            }
            else if (!foundNext)
            {
                break;
            }
        }
        else
        {
            // Continue hanging growth pattern
            int dropDistance = hangingStartHeight - currentPos.y;
            nextPos = getHangingVinePosition(hangingStartPos, currentPos, dropDistance);

            // Check if we hit something
            if (hasWorldVoxel(nextPos))
            {
                // Try to attach to the surface we hit
                if (hasValidAttachment(currentPos))
                {
                    isHanging = false;
                    continue;
                }
                else
                {
                    break;
                }
            }
        }

        // Update position
        currentPos = nextPos;
    }

    // Process branches (similar to main vine but never start hanging sections)
    for (int b = 0; b < branchCount; b++)
    {
        int3 branchPos = branchPoints[b];

        for (int i = 0; i < 8; i++)
        {
            setWorldVoxel(branchPos, 4);

            if (random(branchPos * 4.567) < flowerChance)
            {
                for (int j = 0; j < 6; j++)
                {
                    int3 flowerPos = branchPos + directions[j];
                    if (!hasWorldVoxel(flowerPos))
                    {
                        setWorldVoxel(flowerPos, 6);
                        break;
                    }
                }
            }

            // Find next valid surface position for branch
            bool foundNext = false;
            float bestScore = -1;
            int3 nextPos;

            for (int x = -1; x <= 1; x++)
            {
                for (int y = -1; y <= 1; y++)
                {
                    for (int z = -1; z <= 1; z++)
                    {
                        if (x == 0 && y == 0 && z == 0)
                            continue;

                        int3 testPos = branchPos + int3(x, y, z);
                        if (hasWorldVoxel(testPos))
                            continue;

                        if (hasValidAttachment(testPos))
                        {
                            float score = random(testPos * 5.678);
                            if (score > bestScore)
                            {
                                bestScore = score;
                                nextPos = testPos;
                                foundNext = true;
                            }
                        }
                    }
                }
            }

            if (!foundNext)
                break;
            branchPos = nextPos;
        }
    }

    generateBush(startPos / BRICK_SIZE);
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
        brickOccupancy.part1 = 0;
        brickOccupancy.part2 = 0;
        brickOccupancy.compressed_data_ptr = 0;

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

    if (p.pass == 2)
    {
        bool isSolid = calculateSolid(brickCoord * BRICK_SIZE);
        bool isSolidBelow = calculateSolid(brickCoord * BRICK_SIZE - int3(0, BRICK_SIZE, 0));
        bool isSolidAbove = calculateSolid(brickCoord * BRICK_SIZE + int3(0, BRICK_SIZE, 0));

        // Different noise samples for each vegetation type
        float flowerNoise = snoise(brickCoord / 100.0);
        float bushNoise = snoise(brickCoord * 1.567f);
        float treeNoise = snoise(brickCoord);

        // Generate flowers (at voxel level)
        if (!isSolid && isSolidBelow)
        {
            // Generate multiple flowers per brick
            for (uint z = 0; z < BRICK_SIZE; z += 2)
            {
                for (uint x = 0; x < BRICK_SIZE; x += 2)
                {
                    float sparseRandom = random(float3(brickCoord / 135.03 + int3(x, 0, z) / 12.93));
                    float localNoise = sparseRandom * flowerNoise;
                    if (localNoise > 0.7f || sparseRandom < 0.00001f)
                    {
                        // Randomly choose flower color (materials 8, 9, or 10 for different flowers)
                        uint flowerMaterial = uint(6 + random(int3(x, 0, z)) * 3);
                        generateFlower(brickCoord * BRICK_SIZE + int3(x, 0, z), flowerMaterial);
                    }
                }
            }
        }

        // Generate bushes
        if (bushNoise > 0.9f && !isSolid && isSolidBelow)
        {
            generateBush(brickCoord);
        }

        // Generate trees (original tree code)
        if (treeNoise > 0.93f && !isSolid && isSolidBelow)
        {
            generateTree(brickCoord);
        }

        // Generate vines
        float vineNoise = snoise(brickCoord * 1.9f);
        if (treeNoise > 0.9f &&
            !isSolid &&
            ((!isSolidBelow && isSolidAbove)))
        {
            generateVine(brickCoord * BRICK_SIZE);
        }
    }
}
