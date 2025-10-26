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
// Brick Material Lookup Helpers
//=============================================================================

// Read material ID from palette data (packed as 2 uint16s per uint32)
uint ReadPaletteMaterial(uint paletteIdx) {
    uint wordIdx = paletteIdx / 2;
    uint subIdx = paletteIdx % 2;
    uint packed = p.brickPaletteData.data[wordIdx];
    if (subIdx == 0) {
        return packed & 0xFFFF; // Low 16 bits
    } else {
        return packed >> 16; // High 16 bits
    }
}

// Get material for a voxel in a brick (simplified version - no per-voxel lookup yet)
uint GetBrickVoxelMaterial(uint brickPtr, uint3 localVoxelPos) {
    if (brickPtr >= p.brickCount) {
        return 0; // Invalid brick
    }

    BrickHeader brick = p.brickBuffer.bricks[brickPtr];

    // Calculate voxel index in brick (6x6x6) using XZY ordering to match tree
    uint voxelIdx = localVoxelPos.x + localVoxelPos.z * 6 + localVoxelPos.y * 36;

    // Check occupancy
    if (!brick.TestOccupancy(voxelIdx)) {
        return 0; // Empty voxel
    }

    uint materialCount = brick.MaterialCount;
    uint encodingType = brick.EncodingType;
    uint palettePtr = brick.PalettePtr;

    // For now, just return first material from palette
    // TODO: Implement full per-voxel material lookup
    return ReadPaletteMaterial(palettePtr);
}

//=============================================================================
// Octant Mirroring Traversal - Direct port from VoxelRT
// Reference: VoxelRT Tree64/Map.slang Traversal_OctMirror (line 131-236)
//=============================================================================

// DEBUG: Direct brick traversal (bypassing tree)
TraceResult traceBrickDirect(Ray ray, uint brickPtr) {
    TraceResult result = (TraceResult)0;
    result.hit = false;
    result.distance = ray.maxDist;
    result.iterations = 0;

    // Simple box intersection test in world space
    // Brick bounds: [0, 4) in world coordinates
    float3 boxMin = float3(0, 0, 0);
    float3 boxMax = float3(4, 4, 4);

    // Standard ray-box intersection
    float3 invDir = 1.0 / ray.direction;
    float3 t0 = (boxMin - ray.origin) * invDir;
    float3 t1 = (boxMax - ray.origin) * invDir;

    float3 tNear = min(t0, t1);
    float3 tFar = max(t0, t1);

    float tEnter = max(max(tNear.x, tNear.y), max(tNear.z, 0.0));
    float tExit = min(min(tFar.x, tFar.y), tFar.z);

    // Check if ray intersects box
    if (tEnter < tExit && tExit > 0.0) {
        // Calculate entry position in world space
        float3 entryPos = ray.origin + ray.direction * tEnter;

        // Calculate local position within brick [0, 4)
        float3 localPos = entryPos - boxMin;

        // Map to voxel coordinates [0, 6) for the 6x6x6 brick
        float3 brickVoxelPosFloat = localPos * (6.0 / 4.0);
        int3 voxelPos = int3(clamp(floor(brickVoxelPosFloat), 0.0, 5.0));

        // DDA traversal setup
        int3 step = int3(sign(ray.direction));
        float voxelSize = 4.0 / 6.0; // Each voxel is 0.667 world units

        // Calculate tDelta - how far along the ray to move to cross one voxel
        float3 tDelta = abs(voxelSize / ray.direction);

        // Calculate tSide - distance to next voxel boundary for each axis
        float3 voxelWorldMin = boxMin + float3(voxelPos) * voxelSize;
        float3 nextBoundary = voxelWorldMin + max(float3(step), 0.0) * voxelSize;
        float3 tSide = (nextBoundary - entryPos) / ray.direction;

        // Sphere test: center at (2.5, 2.5, 2.5) in voxel coords, radius 2.5
        float3 sphereCenter = float3(2.5, 2.5, 2.5);
        float sphereRadius = 2.5;

        // Track which axis we last stepped on for normal calculation
        int3 lastStepAxis = int3(0, 0, 0);

        // Traverse voxels using DDA
        for (int step_i = 0; step_i < 18; step_i++) {
            // Check bounds
            if (any(voxelPos < int3(0)) || any(voxelPos >= int3(6))) {
                break; // Exited brick
            }

            // Test if this voxel is inside the sphere
            float3 voxelCenterFloat = float3(voxelPos) + 0.5;
            float dist = length(voxelCenterFloat - sphereCenter);

            if (dist <= sphereRadius) {
                // Hit! Inside sphere
                result.hit = true;
                result.position = boxMin + (float3(voxelPos) + 0.5) * voxelSize;
                result.distance = length(result.position - ray.origin);

                // Encode voxel position for visualization
                result.materialId = 1 + voxelPos.x + voxelPos.y * 10 + voxelPos.z * 100;

                // Normal is based on which face we entered from
                // For first voxel (step_i == 0), use entry normal from box
                // For subsequent voxels, use the last step axis
                if (step_i == 0) {
                    // Entry from outside brick - use box entry normal
                    if (abs(tNear.x - tEnter) < 0.0001) result.normal = float3(-sign(ray.direction.x), 0, 0);
                    else if (abs(tNear.y - tEnter) < 0.0001) result.normal = float3(0, -sign(ray.direction.y), 0);
                    else result.normal = float3(0, 0, -sign(ray.direction.z));
                } else {
                    // Use last step direction - normal points opposite to step
                    result.normal = -float3(lastStepAxis);
                }
                return result;
            }

            // Step to next voxel along the ray and record which axis
            if (tSide.x <= tSide.y && tSide.x <= tSide.z) {
                lastStepAxis = int3(step.x, 0, 0);
                tSide.x += tDelta.x;
                voxelPos.x += step.x;
            } else if (tSide.y <= tSide.z) {
                lastStepAxis = int3(0, step.y, 0);
                tSide.y += tDelta.y;
                voxelPos.y += step.y;
            } else {
                lastStepAxis = int3(0, 0, step.z);
                tSide.z += tDelta.z;
                voxelPos.z += step.z;
            }
        }
    }

    return result;
}

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
    uint stack[11];
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

        // Check for hit - if we're at a leaf, traverse the brick (reference line 191)
        if (BitTestHalf64(node.PopMask, childIdx, 1) && node.IsLeaf && !skipNextHit) {
            // Un-mirror position to get world coordinates
            float3 posUnmirrored = GetMirroredPos(pos, dir, false);
            float3 worldPos = (posUnmirrored - 1.0 - worldOrigin * scale) * (1 << TreeScale);

            // ========== 6×6×6 Brick Traversal ==========
            // Calculate leaf base position in world space (4×4×4 leaf cell)
            float3 leafBase = floor(worldPos / 4.0) * 4.0;
            float3 brickMin = leafBase;
            float3 brickMax = leafBase + 4.0;

            // Intersect ray with brick to get entry point
            float3 invDirBrick = 1.0 / ray.direction;
            float3 t0 = (brickMin - ray.origin) * invDirBrick;
            float3 t1 = (brickMax - ray.origin) * invDirBrick;
            float3 tNear = min(t0, t1);
            float3 tFar = max(t0, t1);
            float tEnter = max(max(tNear.x, tNear.y), max(tNear.z, 0.0));

            // Entry point in world space
            float3 entryPos = ray.origin + ray.direction * tEnter;

            // Local position within brick [0, 4)
            float3 localPos = entryPos - brickMin;

            // Map to 6×6×6 voxel coordinates [0, 6)
            float3 voxelPosFloat = localPos * (6.0 / 4.0);
            int3 voxelPos = int3(clamp(floor(voxelPosFloat), 0.0, 5.0));

            // DDA traversal setup
            int3 stepBrick = int3(sign(ray.direction));
            float voxelSize = 4.0 / 6.0; // Each voxel is 2/3 world unit
            float3 tDelta = abs(voxelSize / ray.direction);

            // Calculate tSide - distance to next voxel boundary for each axis
            float3 voxelWorldMin = brickMin + float3(voxelPos) * voxelSize;
            float3 nextBoundary = voxelWorldMin + max(float3(stepBrick), 0.0) * voxelSize;
            float3 tSide = (nextBoundary - entryPos) / ray.direction;

            // Track last step axis for normal calculation
            int3 lastStepAxis = int3(0, 0, 0);

            // Get brick pointer from leaf node
            uint brickPtr = node.BrickPtr;

            // Track current t value for hit position
            float currentT = tEnter;

            // Traverse the 6×6×6 brick
            for (int step_i = 0; step_i < 18; step_i++) {
                // Check bounds
                if (any(voxelPos < int3(0)) || any(voxelPos >= int3(6))) {
                    break; // Exited brick without hit - continue tree traversal
                }

                // Read actual voxel data from brick using XZY ordering
                uint3 voxelPosU = uint3(voxelPos);
                uint materialId = GetBrickVoxelMaterial(brickPtr, voxelPosU);

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
