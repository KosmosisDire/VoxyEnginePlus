#include <shared.inl>

[[vk::push_constant]]
const ComputePush p;

static const int MAX_STEPS = 1024;

// DDA state structure for raymarching
struct DDAState
{
    int3 cell;        // Current cell position
    int3 raySign;     // Sign of ray direction (1 or -1)
    float3 deltaDist; // Distance between intersections
    float3 sideDist;  // Distance to next intersection
};

// Initialize DDA state for ray traversal
DDAState initDDA(float3 pos, float3 rayDir, float3 invDir)
{
    DDAState state;

    state.raySign = sign(rayDir);
    state.cell = int3(floor(pos));
    state.deltaDist = invDir;
    state.sideDist = (state.cell - pos + 0.5 + state.raySign * 0.5) * state.deltaDist;

    return state;
}

// Get stepping mask for DDA traversal
float3 stepMask(float3 sideDist)
{
    bool3 move;
    bool3 pon = sideDist.xyz < sideDist.yzx;

    move.x = pon.x && !pon.z;
    move.y = pon.y && !pon.x;
    move.z = !(move.x || move.y);

    return float3(move);
}

// Step DDA state and return normal of intersected face
int3 stepDDA(inout DDAState state)
{
    int3 mask = int3(stepMask(state.sideDist));
    int3 normalNeg = mask * state.raySign;

    state.cell += normalNeg;
    state.sideDist += mask * state.raySign * state.deltaDist;

    return -normalNeg; // Returns surface normal
}

bool isInBounds(int3 pos, int3 max)
{
    return all(pos >= float3(0.0)) && all(pos < max);
}

// Ray-box intersection test for grid bounds
bool intersectBox(float3 origin, float3 invDir, float3 boxMin, float3 boxMax, out float tMin, out float tMax)
{
    float3 t0 = (boxMin - origin) * invDir;
    float3 t1 = (boxMax - origin) * invDir;

    float3 tminv = min(t0, t1);
    float3 tmaxv = max(t0, t1);

    tMin = max(max(tminv.x, tminv.y), tminv.z);
    tMax = min(min(tmaxv.x, tmaxv.y), tmaxv.z);

    return tMin <= tMax && tMax > 0.0;
}

bool intersectBoxNormal(float3 origin, float3 invDir, float3 boxMin, float3 boxMax, out float tMin, out float tMax, out float3 normal)
{
    float3 t0 = (boxMin - origin) * invDir;
    float3 t1 = (boxMax - origin) * invDir;

    float3 tminv = min(t0, t1);
    float3 tmaxv = max(t0, t1);

    tMin = max(max(tminv.x, tminv.y), tminv.z);
    tMax = min(min(tmaxv.x, tmaxv.y), tmaxv.z);

    // Initialize normal
    normal = float3(0, 0, 0);

    if (tMin <= tMax && tMax > 0.0)
    {
        // Calculate normal based on which face was hit
        // Use small epsilon for float comparison
        const float epsilon = 0.0001;

        if (abs(tMin - tminv.x) < epsilon)
            normal = float3(sign(invDir.x), 0, 0);
        else if (abs(tMin - tminv.y) < epsilon)
            normal = float3(0, sign(invDir.y), 0);
        else if (abs(tMin - tminv.z) < epsilon)
            normal = float3(0, 0, sign(invDir.z));

        // Ensure normal points outward
        normal = -normal;

        return true;
    }

    return false;
}

// Get chunk index from world position
int chunkIndex(int3 pos)
{
    return pos.x + pos.y * GRID_SIZE + pos.z * GRID_SIZE * GRID_SIZE;
}

// Convert 3D position to linear index
inline int voxelLocalIndex(int3 localVoxelPos)
{
    return localVoxelPos.x + localVoxelPos.y * BRICK_SIZE + localVoxelPos.z * BRICK_SIZE_SQUARE;
}

inline int voxelGlobalIndex(int globalBrickIndex, int localVoxelIndex)
{
    return globalBrickIndex * BRICK_SIZE_CUBE + localVoxelIndex;
}

inline int brickLocalIndex(int3 localBrickPos)
{
    return localBrickPos.x + localBrickPos.y * CHUNK_SIZE + localBrickPos.z * CHUNK_SIZE_SQUARE;
}

inline int brickGlobalIndex(int chunkIndex, int localBrickIndex)
{
    return chunkIndex * CHUNK_SIZE_CUBE + localBrickIndex;
}

// Get adjacent brick index in normal direction
uint getAdjacentBrickIndex(uint brickIndex, int3 normal)
{
    // Get chunk index and local brick index
    uint chunkIdx = brickIndex / CHUNK_SIZE_CUBE;
    uint localBrickIdx = brickIndex % CHUNK_SIZE_CUBE;

    // Convert local brick index to 3D coordinates
    int3 localBrickPos;
    localBrickPos.x = int(localBrickIdx % CHUNK_SIZE);
    localBrickPos.y = int((localBrickIdx / CHUNK_SIZE) % CHUNK_SIZE);
    localBrickPos.z = int(localBrickIdx / CHUNK_SIZE_SQUARE);

    // Convert chunk index to 3D coordinates
    int3 chunkPos;
    chunkPos.x = int(chunkIdx % GRID_SIZE);
    chunkPos.y = int((chunkIdx / GRID_SIZE) % GRID_SIZE);
    chunkPos.z = int(chunkIdx / (GRID_SIZE * GRID_SIZE));

    // Calculate global brick position
    int3 globalBrickPos = chunkPos * CHUNK_SIZE + localBrickPos;

    // Add normal to get adjacent brick position
    // Convert normal to direction vector with integer components
    int3 adjacentGlobalBrickPos = globalBrickPos + normal;

    // Calculate new chunk position (handle negative coordinates correctly)
    int3 newChunkPos;

    // For negative coordinates, we need a different division formula
    // to ensure rounding towards negative infinity
    newChunkPos.x = adjacentGlobalBrickPos.x < 0
                        ? (adjacentGlobalBrickPos.x - CHUNK_SIZE + 1) / CHUNK_SIZE
                        : adjacentGlobalBrickPos.x / CHUNK_SIZE;

    newChunkPos.y = adjacentGlobalBrickPos.y < 0
                        ? (adjacentGlobalBrickPos.y - CHUNK_SIZE + 1) / CHUNK_SIZE
                        : adjacentGlobalBrickPos.y / CHUNK_SIZE;

    newChunkPos.z = adjacentGlobalBrickPos.z < 0
                        ? (adjacentGlobalBrickPos.z - CHUNK_SIZE + 1) / CHUNK_SIZE
                        : adjacentGlobalBrickPos.z / CHUNK_SIZE;

    // Check if the new position is out of bounds
    if (newChunkPos.x < 0 || newChunkPos.y < 0 || newChunkPos.z < 0 ||
        !isInBounds(newChunkPos, int3(GRID_SIZE)))
    {
        // Return invalid index if out of bounds
        return 0xFFFFFFFF; // Sentinel value for invalid index
    }

    // Calculate new local brick position (handle negative coordinates correctly)
    int3 newLocalBrickPos;
    newLocalBrickPos.x = (adjacentGlobalBrickPos.x % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE;
    newLocalBrickPos.y = (adjacentGlobalBrickPos.y % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE;
    newLocalBrickPos.z = (adjacentGlobalBrickPos.z % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE;

    // Calculate new chunk index and local brick index
    int newChunkIdx = chunkIndex(newChunkPos);
    int newLocalBrickIdx = brickLocalIndex(newLocalBrickPos);

    // Return global brick index
    return brickGlobalIndex(newChunkIdx, newLocalBrickIdx);
}

bool brickBitmaskEmpty(BrickBitmask mask)
{
    return all(mask.part1 == 0) && all(mask.part2 == 0);
}

// Get intersection point UVs within current cell
float3 getDdaUvs(DDAState state, float3 rayPos, float3 rayDir)
{
    float3 mini = (float3(state.cell) - rayPos + 0.5 - 0.5 * state.raySign) * state.deltaDist;
    float d = max(mini.x, max(mini.y, mini.z));
    float3 intersect = rayPos + rayDir * d;
    float3 uvs = intersect - float3(state.cell);

    // Handle case when starting inside the initial cell
    if (all(state.cell == int3(floor(rayPos))))
    {
        uvs = rayPos - float3(state.cell);
    }

    return uvs;
}

float3 inverseDirection(float3 dir)
{
    return (1.0 / max(abs(dir), float3(EPSILON))) * sign(dir);
}

void setBit(Ptr<BrickOccupancy> occupancy, int brickIndex, int voxelIndex)
{
    if (voxelIndex < 128)
    {
        InterlockedOr(occupancy.occupancy[brickIndex].part1[voxelIndex / 32], (1 << (voxelIndex % 32)));
    }
    else
    {
        InterlockedOr(occupancy.occupancy[brickIndex].part2[(voxelIndex - 128) / 32], (1 << ((voxelIndex - 128) % 32)));
    }
}

void clearBit(Ptr<BrickOccupancy> occupancy, int brickIndex, int voxelIndex)
{
    if (voxelIndex < 128)
    {
        InterlockedAnd(occupancy.occupancy[brickIndex].part1[voxelIndex / 32], ~(1 << (voxelIndex % 32)));
    }
    else
    {
        InterlockedAnd(occupancy.occupancy[brickIndex].part2[(voxelIndex - 128) / 32], ~(1 << ((voxelIndex - 128) % 32)));
    }
}

void setBit(inout BrickBitmask mask, int index)
{

    if (index < 128)
    {
        mask.part1[index / 32] |= (1 << (index % 32));
    }
    else
    {
        mask.part2[(index - 128) / 32] |= (1 << ((index - 128) % 32));
    }
}

void clearBit(inout BrickBitmask mask, int index)
{
    if (index < 128)
    {
        mask.part1[index / 32] &= ~(1 << (index % 32));
    }
    else
    {
        mask.part2[(index - 128) / 32] &= ~(1 << ((index - 128) % 32));
    }
}

bool getBit(BrickBitmask mask, int index)
{
    // Calculate which 32-bit chunk and bit position
    uint chunk = index / 32;
    uint bitPos = index % 32;
    uint bitMask = 1u << bitPos;

    // Access appropriate part of the mask
    if (chunk < 4)
    {
        return (mask.part1[chunk] & bitMask) != 0;
    }
    else if (chunk < 7) // 216 bits = 6.75 chunks
    {
        return (mask.part2[chunk - 4] & bitMask) != 0;
    }

    return false;
}

bool getBit(daxa_u32vec2 value, int index)
{
    return (bool)(value[index / 32] & (1 << (index % 32)));
}

void setBit(Ptr<ChunkOccupancy> occupancy, int chunkIndex, int brickIndex)
{
    InterlockedOr(occupancy.occupancy[chunkIndex][brickIndex / 32], (1 << (brickIndex % 32)));
}

void clearBit(Ptr<ChunkOccupancy> occupancy, int chunkIndex, int brickIndex)
{
    InterlockedAnd(occupancy.occupancy[chunkIndex][brickIndex / 32], ~(1 << (brickIndex % 32)));
}

Material getMaterial(uint materialID)
{
    if (materialID < 0 || materialID >= TOTAL_MATERIALS)
    {
        materialID = 0;
    }

    return p.materials_ptr.materials[materialID];
}

uint getMaterialId(int2 voxelID)
{
    uint globalVoxelIndex = voxelGlobalIndex(voxelID.x, voxelID.y);
    return (p.voxel_materials_ptr.materials[globalVoxelIndex / 8] >> ((globalVoxelIndex % 8) * 4)) & 0xF;
}

// Calculate per-voxel normal based on surrounding voxel occupancy
float3 calculateVoxelNormal(int brickIndex, int localVoxelIndex, Ptr<BrickOccupancy> occupancy)
{
    // Define the 6 neighbor directions (positive and negative for each axis)
    const int3 neighbors[6] = {
        int3(1, 0, 0), int3(-1, 0, 0), // x-axis neighbors
        int3(0, 1, 0), int3(0, -1, 0), // y-axis neighbors
        int3(0, 0, 1), int3(0, 0, -1)  // z-axis neighbors
    };

    // Convert local voxel index to 3D position within brick
    int3 localVoxelPos;
    localVoxelPos.x = localVoxelIndex % BRICK_SIZE;
    localVoxelPos.y = (localVoxelIndex / BRICK_SIZE) % BRICK_SIZE;
    localVoxelPos.z = localVoxelIndex / BRICK_SIZE_SQUARE;

    // Initialize normal as zero vector
    float3 normal = float3(0.0, 0.0, 0.0);

    // Check each neighbor
    for (int i = 0; i < 6; i++)
    {
        int3 neighborPos = localVoxelPos + neighbors[i];

        // Check if neighbor is outside of current brick
        if (any(neighborPos < 0) || any(neighborPos >= BRICK_SIZE))
        {
            // Calculate which brick this neighbor belongs to
            int3 direction = sign(neighborPos - (BRICK_SIZE - 1) / 2);

            // Only interested in directions where we're out of bounds
            direction.x = (neighborPos.x < 0 || neighborPos.x >= BRICK_SIZE) ? direction.x : 0;
            direction.y = (neighborPos.y < 0 || neighborPos.y >= BRICK_SIZE) ? direction.y : 0;
            direction.z = (neighborPos.z < 0 || neighborPos.z >= BRICK_SIZE) ? direction.z : 0;

            // Get the adjacent brick index
            uint adjacentBrickIndex = getAdjacentBrickIndex(brickIndex, direction);

            // If adjacent brick is invalid, treat as empty space
            if (adjacentBrickIndex == 0xFFFFFFFF)
            {
                // Add contribution to normal (weighted by direction vector)
                normal -= float3(neighbors[i]);
                continue;
            }

            // Wrap around neighbor position to adjacent brick
            neighborPos = (neighborPos + BRICK_SIZE) % BRICK_SIZE;

            // Get local voxel index in the adjacent brick
            int neighborVoxelIndex = voxelLocalIndex(neighborPos);

            // Check if neighbor voxel is empty
            if (!getBit(occupancy.occupancy[adjacentBrickIndex], neighborVoxelIndex))
            {
                // Add contribution to normal (weighted by direction vector)
                normal -= float3(neighbors[i]);
            }
        }
        else
        {
            // Neighbor is in the same brick
            int neighborVoxelIndex = voxelLocalIndex(neighborPos);

            // Check if neighbor voxel is empty
            if (!getBit(occupancy.occupancy[brickIndex], neighborVoxelIndex))
            {
                // Add contribution to normal (weighted by direction vector)
                normal -= float3(neighbors[i]);
            }
        }
    }

    // Normalize the result (if not zero vector)
    return -normalize(normal + float3(EPSILON));
}

// Alternate version using extended 26-neighbor sampling for smoother normals
float3 calculateVoxelNormalSmooth(int brickIndex, int localVoxelIndex, Ptr<BrickOccupancy> occupancy)
{
    // Define weighted neighbor directions for smoother normal calculation
    // We'll use 26 neighbors (8 corners, 12 edges, 6 faces)

    // Convert local voxel index to 3D position within brick
    int3 localVoxelPos;
    localVoxelPos.x = localVoxelIndex % BRICK_SIZE;
    localVoxelPos.y = (localVoxelIndex / BRICK_SIZE) % BRICK_SIZE;
    localVoxelPos.z = localVoxelIndex / BRICK_SIZE_SQUARE;

    // Initialize normal as zero vector
    float3 normal = float3(0.0, 0.0, 0.0);

    // Weights for different types of neighbors
    const float FACE_WEIGHT = 1.0;
    const float EDGE_WEIGHT = 0.7071;   // 1/sqrt(2)
    const float CORNER_WEIGHT = 0.5774; // 1/sqrt(3)

    // Sample all 26 neighbors
    for (int z = -1; z <= 1; z++)
    {
        for (int y = -1; y <= 1; y++)
        {
            for (int x = -1; x <= 1; x++)
            {
                // Skip the center voxel
                if (x == 0 && y == 0 && z == 0)
                    continue;

                int3 offset = int3(x, y, z);
                int3 neighborPos = localVoxelPos + offset;

                // Determine weight based on neighbor type
                float weight;
                if (abs(x) + abs(y) + abs(z) == 1)
                    weight = FACE_WEIGHT;
                else if (abs(x) + abs(y) + abs(z) == 2)
                    weight = EDGE_WEIGHT;
                else
                    weight = CORNER_WEIGHT;

                // Apply direction-weighted contribution
                float3 contribution = float3(offset) * weight;

                // Check if neighbor is outside of current brick
                if (any(neighborPos < 0) || any(neighborPos >= BRICK_SIZE))
                {
                    // Calculate which brick this neighbor belongs to
                    int3 direction = sign(neighborPos - (BRICK_SIZE - 1) / 2);

                    // Only interested in directions where we're out of bounds
                    direction.x = (neighborPos.x < 0 || neighborPos.x >= BRICK_SIZE) ? direction.x : 0;
                    direction.y = (neighborPos.y < 0 || neighborPos.y >= BRICK_SIZE) ? direction.y : 0;
                    direction.z = (neighborPos.z < 0 || neighborPos.z >= BRICK_SIZE) ? direction.z : 0;

                    // Get the adjacent brick index
                    uint adjacentBrickIndex = getAdjacentBrickIndex(brickIndex, direction);

                    // If adjacent brick is invalid, treat as empty space
                    if (adjacentBrickIndex == 0xFFFFFFFF)
                    {
                        // Add contribution to normal
                        normal -= contribution;
                        continue;
                    }

                    // Wrap around neighbor position to adjacent brick
                    neighborPos = (neighborPos + BRICK_SIZE) % BRICK_SIZE;

                    // Get local voxel index in the adjacent brick
                    int neighborVoxelIndex = voxelLocalIndex(neighborPos);

                    // Check if neighbor voxel is empty
                    if (!getBit(occupancy.occupancy[adjacentBrickIndex], neighborVoxelIndex))
                    {
                        // Add contribution to normal
                        normal -= contribution;
                    }
                }
                else
                {
                    // Neighbor is in the same brick
                    int neighborVoxelIndex = voxelLocalIndex(neighborPos);

                    // Check if neighbor voxel is empty
                    if (!getBit(occupancy.occupancy[brickIndex], neighborVoxelIndex))
                    {
                        // Add contribution to normal
                        normal -= contribution;
                    }
                }
            }
        }
    }

    // Normalize the result (if not zero vector)
    return -normalize(normal + float3(EPSILON));
}
