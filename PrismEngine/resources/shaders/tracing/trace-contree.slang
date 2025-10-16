#pragma once

//=============================================================================
// Tree64 Traversal - Direct port from VoxelRT
// Reference: VoxelRT/src/VoxelRT/Shaders/Backends/Tree64/Map.slang
//=============================================================================

// Toggle shared memory optimization
// Set to 1 to use groupshared memory (faster, VoxelRT's approach)
// Set to 0 to use per-thread registers (slower, simpler)
#define USE_SHARED_MEMORY_STACK 0

// Get child index from position using bit manipulation (XZY ordering)
static int GetNodeCellIndex(float3 pos, int scale_exp) {
    uint3 cellPos = asuint(pos) >> scale_exp & 3;
    return cellPos.x + cellPos.z * 4 + cellPos.y * 16;  // XZY ordering!
}

// Floor position to scale boundary
static float3 FloorScale(float3 pos, int scale_exp) {
    uint mask = ~0u << scale_exp;
    return asfloat(asuint(pos) & mask);
}

// Count set bits before index (popcount with mask)
// Reference: VoxelRT BackendCommon.slang prefix_popcnt64
inline uint prefix_popcnt64(uint64_t mask, uint width) {
    uint himask = uint(mask);  // Low 32 bits
    uint count = 0;

    if (width >= 32) {
        count = countbits(himask);
        himask = uint(mask >> 32);  // High 32 bits
    }
    uint m = 1u << (width & 31u);
    count += countbits(himask & (m - 1u));

    return count;
}

// Cheaper approximation of `(value >> shift & mask) != 0`
// Reference: VoxelRT BackendCommon.slang BitTestHalf64
static bool BitTestHalf64(uint64_t value, uint shift, uint mask) {
    uint low = shift < 32 ? uint(value) : uint(value >> 32);
    return (low >> (shift & 31) & mask) != 0;
}

// Computes `dir > 0 ? 3.0 - pos : pos` with special case to avoid rounding
// Reference: VoxelRT Tree64/Map.slang GetMirroredPos (line 238-246)
static float3 GetMirroredPos(float3 pos, float3 dir, bool rangeCheck) {
    float3 mirrored = asfloat(asuint(pos) ^ 0x7FFFFF);
    // XOR'ing will only work if the coords are in range [1.0, 2.0),
    // so just fallback to the original subtraction if that's not the case.
    if (rangeCheck && any(pos < 1.0 || pos >= 2.0)) mirrored = 3.0 - pos;
    return select(dir > 0, mirrored, pos);
}

//=============================================================================
// Octant Mirroring Traversal - Direct port from VoxelRT
// Reference: VoxelRT Tree64/Map.slang Traversal_OctMirror (line 131-236)
//=============================================================================

TraceResult traceContree(Ray ray, uint groupThreadIndex, bool coarse = false) {
#if USE_SHARED_MEMORY_STACK
    // Shared memory for traversal stacks (VoxelRT optimization)
    // Reference: VoxelRT Tree64/Map.slang line 136
    // Each workgroup thread gets its own stack in shared memory for better cache locality
    static groupshared uint gs_stack[64][11];  // 64 threads per workgroup, 11 levels deep
#endif

    TraceResult result = (TraceResult)0;
    result.hit = false;
    result.distance = ray.maxDist;
    result.iterations = 0;

    // STEP 1: Intersect ray with world-space bounding box [0, 256)^3
    float3 boxMin = float3(0, 0, 0);
    float3 boxMax = float3(256, 256, 256);

    float3 invDirWorld = 1.0 / max(abs(ray.direction), float3(EPSILON));
    invDirWorld *= sign(ray.direction);

    float tMin, tMax;
    float3 hitNormal;
    if (!intersectBoxNormal(ray.origin, invDirWorld, boxMin, boxMax, tMin, tMax, hitNormal)) {
        return result; // No intersection with tree volume
    }

    if (tMin > ray.maxDist || tMax < 0) {
        return result; // Too far or behind camera
    }

    // STEP 2: Transform to [1.0, 2.0) normalized space
    // Reference VoxelRT line 26-29
    const uint TreeScale = 8;  // 2^8 = 256
    float scale = 1.0 / (1 << TreeScale);  // 1.0 / 256.0

    // For single tree at world origin (0,0,0):
    int3 worldOrigin = int3(0, 0, 0);
    float3 origin = worldOrigin * scale + ray.origin * scale + 1.0;
    float3 dir = ray.direction;

    // Stack allocation based on optimization mode
#if USE_SHARED_MEMORY_STACK
    // Shared memory optimization (VoxelRT line 138)
    uint groupId = groupThreadIndex;  // 0-63 for 8x8 workgroup (passed as parameter)
    uint* stack = &gs_stack[groupId][0];
#else
    // Register-based stack (fallback, simpler but slower)
    uint localStack[11];
    uint* stack = &localStack[0];
#endif
    int scaleExp = 21;  // Always start at 21 for finest level (0.25 = 2^-2)

    uint nodeIdx = 0; // root
    ContreeNode node = p.contreeBuffer.nodes[nodeIdx];

    // Octant mirroring setup (reference line 144-150)
    float3 invDir = 1.0f / -abs(dir);
    uint mirrorMask = 0;

    // Mirror coordinates to negative ray octant
    if (dir.x > 0) mirrorMask |= 3 << 0;
    if (dir.y > 0) mirrorMask |= 3 << 4;
    if (dir.z > 0) mirrorMask |= 3 << 2;

    origin = GetMirroredPos(origin, dir, true);
    float3 pos = clamp(origin, 1.0f, 1.9999999f);

    float3 sideDist;
    int childIdx;
    int i;
    bool skipNextHit = true; // true on first iteration so that self-intersections are ignored

    // Clip rays starting outside tree (reference line 160-175)
    if (any(pos != origin)) {
        float3 t0 = (float3(2.0f) - origin) * invDir;
        float3 t1 = (float3(1.0f) - origin) * invDir;
        float tmin = max(max(t0.x, t0.y), max(t0.z, 0.0f));
        float tmax = min(min(t1.x, t1.y), t1.z);

        pos = clamp(origin - abs(dir) * tmin, 1.0f, 1.9999999f);
        sideDist = -t0; // negation corrects for normal calculation

        // The first hit may be valid if we start outside the tree and
        // there are voxels at the edges.
        skipNextHit = tmin >= tmax;
    }

    // Main traversal loop (reference line 177-219)
    for (i = 0; i < ray.maxIterations; i++) {
        // Coarse early-out for shadow rays (reference line 178)
        // After 20 iterations, if we're in a leaf, we hit something - good enough for shadows!
        if (coarse && i > 20 && node.IsLeaf) break;

        childIdx = GetNodeCellIndex(pos, scaleExp) ^ mirrorMask;

        // Descend (reference line 183-190)
        while (BitTestHalf64(node.PopMask, childIdx, 1) && !node.IsLeaf) {
            stack[scaleExp >> 1] = nodeIdx;
            nodeIdx = node.ChildPtr + prefix_popcnt64(node.PopMask, childIdx);

            if (nodeIdx >= p.contreeNodeCount) break;
            node = p.contreeBuffer.nodes[nodeIdx];

            scaleExp -= 2;
            childIdx = GetNodeCellIndex(pos, scaleExp) ^ mirrorMask;
        }

        // Check for hit (reference line 191)
        if (BitTestHalf64(node.PopMask, childIdx, 1) && node.IsLeaf && !skipNextHit) break;

        int advScaleExp = scaleExp;

        // 2³ anisotropic stepping (reference line 196)
        if (!BitTestHalf64(node.PopMask, childIdx & 0b101010, 0x00330033)) advScaleExp++;

        // Compute next pos by intersecting with max cube edges (reference line 199-205)
        float3 edgePos = FloorScale(pos, advScaleExp);

        sideDist = (edgePos - origin) * invDir;
        float tmax = min(min(sideDist.x, sideDist.y), sideDist.z);

        int3 maxSiblBounds = asuint(edgePos) + select(sideDist == tmax, -1, (1 << advScaleExp) - 1);
        pos = min(origin - abs(dir) * tmax, asfloat(maxSiblBounds));

        // Find carry bit to tell which node we should ascend to (reference line 207-217)
        uint3 diffPos = asuint(pos) ^ asuint(edgePos);
        int diffExp = firstbithigh((diffPos.x | diffPos.y | diffPos.z) & 0xFFAAAAAA); // 31 - lzcnt

        if (diffExp > scaleExp) {
            scaleExp = diffExp;
            if (diffExp > 21) break;  // going out of root?

            nodeIdx = stack[scaleExp >> 1];
            if (nodeIdx >= p.contreeNodeCount) break;
            node = p.contreeBuffer.nodes[nodeIdx];
        }
        skipNextHit = false;
    }

    result.iterations = i;

    // Process hit (reference line 225-234)
    if (node.IsLeaf && scaleExp <= 21) {
        // Un-mirror position (reference line 226)
        pos = GetMirroredPos(pos, dir, false);

        result.hit = true;
        result.position = (pos - 1.0 - worldOrigin * scale) * (1 << TreeScale);
        result.distance = length(result.position - ray.origin);

        // Material ID (would need ReadMaterialId function like VoxelRT)
        result.materialId = 1; // Placeholder

        // Calculate normal (reference line 231-233)
        float tmax = min(min(sideDist.x, sideDist.y), sideDist.z);
        bool3 sideMask = tmax >= sideDist;
        result.normal = select(sideMask, -sign(dir), 0.0);
    }

    return result;
}
