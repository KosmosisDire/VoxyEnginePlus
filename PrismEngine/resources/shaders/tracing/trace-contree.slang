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
// Brick Material Lookup Helpers (following structure.md)
//=============================================================================

// Get material for a voxel in a brick
// Reference: structure.md "Step 3: Fetch Material Data"
uint GetBrickVoxelMaterial(ContreeNode leafNode, uint3 localVoxelPos)
{
    // CRITICAL: Verify this is actually a leaf node!
    // If IsLeaf is false, we're reading PackedData[1] as PopMask (0xFFFFFFFF), not PalettePtr (0)
    if (!leafNode.IsLeaf) {
        return 0; // Not a leaf - shouldn't happen but defend against it
    }

    // Step 1: Calculate voxel index in 12x12x12 brick using XYZ ordering
    // Index = x + y*BRICK_SIZE + z*BRICK_SIZE*BRICK_SIZE
    uint voxelIdx = localVoxelPos.x + localVoxelPos.y * BRICK_SIZE + localVoxelPos.z * (BRICK_SIZE * BRICK_SIZE);

    if (voxelIdx >= BRICK_VOXELS) {
        return 0; // Out of bounds
    }

    // Step 2: Get brick occupancy (SHARED - multiple leaves may point to same brick)
    uint brickPtr = leafNode.OccupancyPtr;
    if (brickPtr >= p.brickCount) {
        return 0; // Invalid brick
    }

    Brick brick = p.brickBuffer.bricks[brickPtr];

    // Step 3: Test occupancy
    if (!brick.TestOccupancy(voxelIdx)) {
        return 0; // Empty voxel
    }

    // Step 4: Get palette pointer (SHARED - multiple leaves may use same palette)
    uint palettePtr = leafNode.PalettePtr;

    // Step 5: Read material count from palette
    uint materialCount = p.paletteBuffer.data[0];

    // Step 6: Get material indices pointer (SHARED - all leaves point to same checkerboard)
    uint indicesPtr = leafNode.MaterialIndicesPtr;

    // Step 7: Read material index for this voxel (1728 uint32s for 12x12x12)
    uint materialIdx = p.materialIndicesBuffer.data[indicesPtr + voxelIdx];

    // Step 8: Look up actual material ID from palette (skip count at palettePtr)
    uint materialId = p.paletteBuffer.data[palettePtr + 1 + materialIdx];

    return materialId;
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
    // Use SPIR-V inline assembly to get thread index and enable VariablePointers capability
    uint groupId = spirv_asm {
        OpCapability VariablePointers;
        result:$$uint = OpLoad builtin(LocalInvocationIndex:uint);
    };
    uint* stack = &gs_stack[groupId][0];
#else
    // Register-based stack (no pointers, direct array indexing)
    uint stack[15];
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
        if (coarse && i > 100 && node.IsLeaf) break;

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

        // Check for hit - if we're at a leaf, traverse the brick
        // NOTE: For leaf nodes, PopMask is NOT valid! PackedData[1-2] store pointers instead.
        // Just check IsLeaf (the parent's PopMask already confirmed this child exists)
        if (node.IsLeaf && !skipNextHit) {
            // Un-mirror position to get world coordinates
            float3 posUnmirrored = GetMirroredPos(pos, dir, false);
            float3 worldPos = (posUnmirrored - 1.0 - worldOrigin * scale) * (1 << TreeScale);

            // ========== 12×12×12 Brick Traversal ==========
            // Each leaf covers a 4×4×4 world space region (from contree structure)
            // We pack 12×12×12 voxels into that 4×4×4 space
            // So each voxel is 4.0/12.0 = 0.333... world units
            float3 leafBase = floor(worldPos / float(CONTREE_BRANCHING)) * float(CONTREE_BRANCHING);  // Leaf is 4×4×4 world units
            float3 brickMin = leafBase;
            float3 brickMax = leafBase + float(CONTREE_BRANCHING);  // Not BRICK_SIZE! The leaf is 4 units, not 12

            // Intersect ray with brick to get entry point
            float3 invDirBrick = 1.0 / ray.direction;
            float3 t0 = (brickMin - ray.origin) * invDirBrick;
            float3 t1 = (brickMax - ray.origin) * invDirBrick;
            float3 tNear = min(t0, t1);
            float3 tFar = max(t0, t1);
            float tEnter = max(max(tNear.x, tNear.y), max(tNear.z, 0.0));

            // Entry point in world space
            float3 entryPos = ray.origin + ray.direction * tEnter;

            // Local position within brick [0, 4) - the leaf covers 4×4×4 world units
            float3 localPos = entryPos - brickMin;

            // Map to 12×12×12 voxel coordinates [0, BRICK_SIZE)
            // We pack 12 voxels into 4 world units, so each voxel is 4.0/12.0 world units
            const float voxelSize = float(CONTREE_BRANCHING) / float(BRICK_SIZE);  // 4.0/12.0 = 0.333...
            float3 voxelPosFloat = localPos / voxelSize;  // Scale from [0,4) to [0,12)
            int3 voxelPos = int3(clamp(floor(voxelPosFloat), 0.0, float(BRICK_SIZE - 1)));

            // DDA traversal setup
            int3 stepBrick = int3(sign(ray.direction));
            float3 tDelta = abs(voxelSize / ray.direction);

            // Calculate tSide - distance to next voxel boundary for each axis
            float3 voxelWorldMin = brickMin + float3(voxelPos) * voxelSize;
            float3 nextBoundary = voxelWorldMin + max(float3(stepBrick), 0.0) * voxelSize;
            float3 tSide = (nextBoundary - entryPos) / ray.direction;

            // Track last step axis for normal calculation
            int3 lastStepAxis = int3(0, 0, 0);

            // Track current t value for hit position
            float currentT = tEnter;

            // var posInt = floor(voxelPos);
            // result.materialId = (((int)posInt.x) + ((int)(posInt.y * 4)) + ((int)(posInt.z * 4 * 4)));
            // result.position = ray.origin + ray.direction * currentT;
            // result.distance = currentT;
            // result.normal = float3(0, 1, 0);
            // result.hit = true;

            // return result;

            // Traverse the 12×12×12 brick
            // Worst case: diagonal traversal = sqrt(3) * BRICK_SIZE ≈ 20.78, round up to 24
            for (int step_i = 0; step_i < ceil(1.732 * float(BRICK_SIZE)) + 1; step_i++) {
                // Check bounds
                if (any(voxelPos < int3(0)) || any(voxelPos >= int3(BRICK_SIZE))) {
                    break; // Exited brick without hit - continue tree traversal
                }

                // Read actual voxel data from brick using XYZ ordering
                uint3 voxelPosU = uint3(voxelPos);
                uint materialId = GetBrickVoxelMaterial(node, voxelPosU);

                if (materialId != 0) {
                    // Hit! Found a solid voxel
                    result.hit = true;

                    // Use actual ray hit position (not voxel center!)
                    result.position = ray.origin + ray.direction * currentT;
                    result.distance = currentT;

                    // Use actual material ID from brick data
                    result.materialId = materialId;
                    result.iterations = i;

                    // Calculate normal based on which face we entered the voxel from
                    if (step_i == 0) {
                        // First voxel - use brick entry normal (face we entered the brick from)
                        if (abs(tNear.x - tEnter) < 0.0001) {
                            result.normal = float3(-sign(ray.direction.x), 0, 0);
                        } else if (abs(tNear.y - tEnter) < 0.0001) {
                            result.normal = float3(0, -sign(ray.direction.y), 0);
                        } else {
                            result.normal = float3(0, 0, -sign(ray.direction.z));
                        }
                    } else {
                        // Subsequent voxels - normal points opposite to the step direction
                        result.normal = -float3(lastStepAxis);
                    }

                    return result; // EARLY RETURN - we hit a voxel!
                }

                // Step to next voxel along the ray and update currentT
                if (tSide.x <= tSide.y && tSide.x <= tSide.z) {
                    lastStepAxis = int3(stepBrick.x, 0, 0);
                    currentT = tEnter + tSide.x;  // Update t for next voxel
                    tSide.x += tDelta.x;
                    voxelPos.x += stepBrick.x;
                } else if (tSide.y <= tSide.z) {
                    lastStepAxis = int3(0, stepBrick.y, 0);
                    currentT = tEnter + tSide.y;  // Update t for next voxel
                    tSide.y += tDelta.y;
                    voxelPos.y += stepBrick.y;
                } else {
                    lastStepAxis = int3(0, 0, stepBrick.z);
                    currentT = tEnter + tSide.z;  // Update t for next voxel
                    tSide.z += tDelta.z;
                    voxelPos.z += stepBrick.z;
                }
            }

            // If we reach here, we didn't hit anything in the brick - continue tree traversal
        }

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

    // If we exited the loop without hitting anything, return miss
    return result;
}
