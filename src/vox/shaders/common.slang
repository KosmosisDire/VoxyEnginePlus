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

inline float3 brickWorldPos(int brickIndex)
{
    int chunkIndex = brickIndex / CHUNK_SIZE_CUBE;
    int localBrickIndex = brickIndex % CHUNK_SIZE_CUBE;

    int3 chunkPos = int3(
        chunkIndex % GRID_SIZE,
        (chunkIndex / GRID_SIZE) % GRID_SIZE,
        chunkIndex / (GRID_SIZE * GRID_SIZE));

    int3 localBrickPos = int3(
        localBrickIndex % CHUNK_SIZE,
        (localBrickIndex / CHUNK_SIZE) % CHUNK_SIZE,
        localBrickIndex / CHUNK_SIZE_SQUARE);

    return float3(chunkPos + localBrickPos * WORLD_BRICK_SIZE);
}

// Get adjacent brick index in normal direction
uint getAdjacentBrickIndex(uint brickIndex, float3 normal)
{
    int3 offset = int3(round(normal));
    int3 basePos = int3(brickWorldPos(brickIndex) / WORLD_BRICK_SIZE);
    int3 neighborPos = basePos + offset;

    uint neighborChunkIndex = chunkIndex(neighborPos / CHUNK_SIZE);
    uint neighborLocalIndex = brickLocalIndex(neighborPos % CHUNK_SIZE);
    return brickGlobalIndex(neighborChunkIndex, neighborLocalIndex);
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
