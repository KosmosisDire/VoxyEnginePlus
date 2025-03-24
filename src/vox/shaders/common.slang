#include <shared.inl>

[[vk::push_constant]]
const ComputePush p;

static const int MAX_STEPS = 1024;

float3 inverseDirection(float3 dir)
{
    return (1.0 / max(abs(dir), float3(EPSILON))) * sign(dir);
}

void setBit(Ptr<BrickOccupancy> occupancy, int brickIndex, int voxelIndex)
{
    InterlockedOr(occupancy.occupancy[brickIndex].data[voxelIndex / 32], (1 << (voxelIndex % 32)));
}

void clearBit(Ptr<BrickOccupancy> occupancy, int brickIndex, int voxelIndex)
{
    InterlockedAnd(occupancy.occupancy[brickIndex].data[voxelIndex / 32], ~(1 << (voxelIndex % 32)));
}

void setBit(inout BrickBitmask mask, int index)
{

    mask.data[index / 32] |= (1 << (index % 32));
}

void clearBit(inout BrickBitmask mask, int index)
{
    mask.data[index / 32] &= ~(1 << (index % 32));
}

bool getBit(BrickBitmask mask, int index)
{
    // Calculate which 32-bit chunk and bit position
    uint chunk = index / 32;
    uint bitPos = index % 32;
    uint bitMask = 1u << bitPos;

    // Return true if bit is set
    return (mask.data[chunk] & bitMask) != 0;
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


// Structure to hold DDA traversal state
struct DDAState
{
    int3 cell;      // Current cell indices
    int3 step;      // Step direction (1 or -1) for each axis
    float3 tDelta;  // How far to move along ray to traverse one cell in each axis
    float3 tMax;    // Distance to next cell boundary
};

// Initialize DDA state for traversal
DDAState initDDA(float3 pos, float3 rayDir, float3 invDir)
{
    DDAState state;

    state.step = sign(rayDir);
    state.cell = int3(floor(pos));
    state.tDelta = invDir;
    state.tMax = (state.cell - pos + 0.5 + state.step * 0.5) * state.tDelta;

    return state;
}

// Get stepping mask for DDA traversal
float3 stepMask(float3 tMax)
{
    bool3 move;
    bool3 pon = tMax.xyz < tMax.yzx;

    move.x = pon.x && !pon.z;
    move.y = pon.y && !pon.x;
    move.z = !(move.x || move.y);

    return float3(move);
}

// Step DDA state and return normal of intersected face
int3 stepDDA(inout DDAState state)
{
    int3 mask = int3(stepMask(state.tMax));
    int3 normalNeg = mask * state.step;

    state.cell += normalNeg;
    state.tMax += mask * state.step * state.tDelta;

    return -normalNeg; // Returns surface normal
}

// Helper functions for bit operations
uint countLeadingZeros(uint x)
{
    return firstbithigh(x) ^ 31;
}

// Fast bit test for chunk mask
bool getBitFast(daxa_u32vec2 mask, int index)
{
    return (mask[index >> 5] & (1u << (index & 31))) != 0;
}

// Fast bit test for brick mask
bool getBitFast(BrickBitmask mask, int index)
{
    return (mask.data[index >> 5] & (1u << (index & 31))) != 0;
}

// Check if a brick mask is empty (all bits are 0)
bool brickBitmaskEmpty(BrickBitmask mask)
{
    // Check if all data elements are 0
    for (int i = 0; i < 8; i++)
    {
        if (mask.data[i] != 0) return false;
    }
    return true;
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
    // Extract chunk and local brick indices
    uint chunkIdx = brickIndex / CHUNK_SIZE_CUBE;
    uint localBrickIdx = brickIndex % CHUNK_SIZE_CUBE;
    
    // Convert to 3D positions
    int3 chunkPos;
    chunkPos.x = int(chunkIdx % GRID_SIZE);
    chunkPos.y = int((chunkIdx / GRID_SIZE) % GRID_SIZE);
    chunkPos.z = int(chunkIdx / (GRID_SIZE * GRID_SIZE));
    
    int3 localPos;
    localPos.x = int(localBrickIdx % CHUNK_SIZE);
    localPos.y = int((localBrickIdx / CHUNK_SIZE) % CHUNK_SIZE);
    localPos.z = int(localBrickIdx / CHUNK_SIZE_SQUARE);
    
    // Calculate global position and add offset
    int3 globalPos = chunkPos * CHUNK_SIZE + localPos + normal;
    
    // Calculate new chunk position using floor division
    int3 newChunkPos = int3(floor(float3(globalPos) / float(CHUNK_SIZE)));
    
    // Check bounds
    if (any(newChunkPos < 0) || any(newChunkPos >= GRID_SIZE))
    {
        return 0xFFFFFFFF; // Invalid index
    }
    
    // Calculate new local position (this will always be positive now)
    int3 newLocalPos = globalPos - newChunkPos * CHUNK_SIZE;
    
    // Calculate new indices
    int newChunkIdx = chunkIndex(newChunkPos);
    int newLocalIdx = brickLocalIndex(newLocalPos);
    
    return brickGlobalIndex(newChunkIdx, newLocalIdx);
}

// Get intersection point UVs within current cell
float3 getDdaUvs(DDAState state, float3 rayPos, float3 rayDir)
{
    float3 mini = (float3(state.cell) - rayPos + 0.5 - 0.5 * state.step) * state.tDelta;
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

uint getMaterialId(int2 voxelID)
{
    uint globalVoxelIndex = voxelGlobalIndex(voxelID.x, voxelID.y);
    return (p.voxel_materials_ptr.materials[globalVoxelIndex / 8] >> ((globalVoxelIndex % 8) * 4)) & 0xF;
}
