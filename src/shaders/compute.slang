#include "atmosphere.slang"
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

struct TraceResult
{
    bool hit;
    float3 hitPos;
    float3 normal;
    float distance;
    float3 uvs;
};

#define continueDDA(state)   \
    normal = stepDDA(state); \
    steps++;                 \
    continue;

TraceResult traceVoxelRay(float3 origin, float3 direction)
{
    float3 trueStart = origin;
    float3 invDir = inverseDirection(direction);

    TraceResult result;
    result.hit = false;
    result.hitPos = float3(0.0);
    result.normal = float3(0.0);
    result.distance = 0.0;

    // intersect with bounding box
    float tMin, tMax;
    float3 normal;
    bool boxHit = intersectBoxNormal(origin, invDir, float3(0.0), float3(TREE_SIZE), tMin, tMax, normal);
    if (!boxHit)
    {
        return result;
    }

    // adjust origin to be inside the box
    if (tMin > 0.0)
    {
        origin += direction * (tMin + EPSILON);
    }

    int steps = 0;

    uint64_t chunkMask = p.chunk_occupancy_ptr.occupancy[0];

    // Initialize DDA state
    DDAState brickDDA = initDDA(origin, direction, invDir);
    while (steps < MAX_STEPS && isInBounds(brickDDA.cell, int3(TREE_SIZE)))
    {
        int blockIdx = treeLocalIndex(brickDDA.cell);
        if (getBit(chunkMask, blockIdx))
        {
            int blockIdxGlobal = treeGlobalIndex(0, blockIdx);
            uint64_t brickMask = p.brick_occupancy_ptr.occupancy[blockIdxGlobal];

            if (brickMask == 0)
            {
                continueDDA(brickDDA);
            }

            float3 brickUv = getDdaUvs(brickDDA, origin, direction);
            float3 voxelOrigin = clamp(brickUv * TREE_SIZE, float3(EPSILON), float3(TREE_SIZE - EPSILON));

            DDAState voxelDDA = initDDA(voxelOrigin, direction, invDir);
            while (steps < MAX_STEPS && isInBounds(voxelDDA.cell, int3(TREE_SIZE)))
            {
                int voxelIdx = treeLocalIndex(voxelDDA.cell);

                if (getBit(brickMask, voxelIdx))
                {
                    float3 voxelUv = getDdaUvs(voxelDDA, voxelOrigin, direction);
                    int3 absoluteCell = brickDDA.cell * TREE_SIZE + voxelDDA.cell;
                    float3 hitPos = (float3(absoluteCell) + voxelUv) / float(TREE_SIZE);

                    result.hit = true;
                    result.hitPos = hitPos;
                    result.normal = normal;
                    result.distance = length(hitPos - trueStart);
                    result.uvs = voxelUv;

                    return result;
                }

                continueDDA(voxelDDA);
            }
        }

        continueDDA(brickDDA);
    }

    return result;
}

void getRayFromPixel(float2 pixel, uint2 dimensions, CameraData camera, out float3 rayOrigin, out float3 rayDir)
{
    // Convert pixel to NDC space (-1 to 1)
    float2 uv = (pixel + 0.5f) / float2(dimensions);
    float2 ndc = uv * 2.0f - 1.0f;

    // Inverse projection to get view space position
    float4 viewSpace = mul(camera.invViewProj, float4(ndc.x, ndc.y, 1.0f, 1.0f));
    viewSpace /= viewSpace.w;

    // Get ray direction in world space
    rayOrigin = camera.position;
    rayDir = normalize(viewSpace.xyz - camera.position);
}

float4 alphaBlend(float4 src, float4 dst)
{
    return src + dst * (1.0 - src.w);
}

void setPixel(float4 color, uint2 pixel, RWTexture2D<float4> render_image)
{
    if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
        return;

    render_image[pixel] = alphaBlend(color, render_image[pixel]);
}

void setPixel(float3 color, uint2 pixel, RWTexture2D<float4> render_image)
{
    if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
        return;

    render_image[pixel] = float4(color, 1.0);
}

[numthreads(8, 8, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    RWTexture2D<float4> render_image = RWTexture2D<float4>::get(p.image);
    uint2 pixel = pixel_i.xy;
    if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
        return;

    // Ray setup
    float3 rayOrigin, rayDir;
    getRayFromPixel(pixel, p.frame_dim, p.state_ptr.camera, rayOrigin, rayDir);

    TraceResult trace = traceVoxelRay(rayOrigin, rayDir);

    // lighting
    if (trace.hit)
    {
        TraceResult shadowTrace = traceVoxelRay(trace.hitPos + trace.normal * EPSILON, p.state_ptr.sunDir);
        float shadow = 1.0;
        if (shadowTrace.hit)
        {
            shadow = 0.0;
        }

        float3 normal = normalize(trace.normal);
        float3 color = float3(0.0);

        // simple lambertian shading
        float3 sunDir = normalize(p.state_ptr.sunDir);
        float lDot = dot(normal, sunDir);
        float lightIntensity = max(lDot * shadow, 0.1);

        color = getSunColor(sunDir) * p.state_ptr.backgroundColor * lightIntensity;

        // apply shading
        setPixel(color, pixel, render_image);
    }
    else
    {
        // set background color
        float3 sky = getAtmosphereColor(rayDir, p.state_ptr.sunDir, p.time);
        setPixel(sky, pixel, render_image);
    }
}
