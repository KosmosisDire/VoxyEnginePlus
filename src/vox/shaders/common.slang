#include <shared.inl>

[[vk::push_constant]]
const ComputePush p;

static const float EPSILON = 0.001;
static const int MAX_STEPS = 512;

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

// Convert 3D position to linear index
inline int treeLocalIndex(int3 pos)
{
    return pos.x + pos.y * TREE_SIZE + pos.z * TREE_SIZE * TREE_SIZE;
}

inline int treeGlobalIndex(int parentIndex, int localIndex)
{
    return parentIndex * TREE_SIZE_CUBE + localIndex;
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

bool getBit(uint64_t value, int index)
{
    return (value & (1 << index)) != 0;
}
