#include <shared.inl>

[[vk::push_constant]]
const ComputePush p; 

static const int MAX_STEPS = 1024;

float3 inverseDirection(float3 dir)
{
    return (1.0 / max(abs(dir), float3(EPSILON))) * sign(dir);
}

Material getMaterial(uint materialID)
{
    if (materialID < 0 || materialID >= TOTAL_MATERIALS)
    {
        materialID = 0;
    }

    return p.materialsBuffer.data[materialID];
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
    normal = 0;

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
