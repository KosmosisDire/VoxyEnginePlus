#include "contree.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
#include <bitset>
#include <unordered_map>

namespace Contree {

//=============================================================================
// Brick Builder Implementation
//=============================================================================

uint8_t BrickBuilder::CountUniqueMaterials(const BrickData& brick, std::vector<uint16_t>& uniqueMaterials) {
    uniqueMaterials.clear();
    std::unordered_map<uint16_t, bool> seen;

    for (int i = 0; i < 216; i++) {
        uint16_t mat = brick.materials[i];
        if (mat != 0 && seen.find(mat) == seen.end()) {
            seen[mat] = true;
            uniqueMaterials.push_back(mat);
        }
    }

    return static_cast<uint8_t>(uniqueMaterials.size());
}

BrickHeader BrickBuilder::BuildBrick(
    const BrickData& brick,
    std::vector<uint16_t>& paletteDataOut,
    std::vector<uint8_t>& indexDataOut)
{
    BrickHeader header;

    // Step 1: Build occupancy mask
    for (int i = 0; i < 216; i++) {
        if (brick.materials[i] != 0) {
            header.SetOccupancy(i, true);
        }
    }

    // Step 2: Count unique materials
    std::vector<uint16_t> uniqueMaterials;
    uint8_t materialCount = CountUniqueMaterials(brick, uniqueMaterials);

    header.SetMaterialCount(materialCount);

    // Step 3: Choose encoding based on material count
    if (materialCount == 0) {
        // Empty brick
        header.SetEncodingType(BRICK_SINGLE_MATERIAL);
        header.SetPalettePtr(0);
        return header;
    }
    else if (materialCount == 1) {
        // Single material - no per-voxel data needed
        header.SetEncodingType(BRICK_SINGLE_MATERIAL);
        header.SetPalettePtr(static_cast<uint16_t>(paletteDataOut.size()));
        paletteDataOut.push_back(uniqueMaterials[0]);
        return header;
    }
    else if (materialCount == 2) {
        header.SetEncodingType(BRICK_PALETTE_1BIT);
    }
    else if (materialCount <= 4) {
        header.SetEncodingType(BRICK_PALETTE_2BIT);
    }
    else if (materialCount <= 16) {
        header.SetEncodingType(BRICK_PALETTE_4BIT);
    }
    else {
        header.SetEncodingType(BRICK_PALETTE_8BIT);
    }

    // Step 4: Build material palette
    header.SetPalettePtr(static_cast<uint16_t>(paletteDataOut.size()));
    std::unordered_map<uint16_t, uint8_t> materialToIndex;

    for (size_t i = 0; i < uniqueMaterials.size(); i++) {
        paletteDataOut.push_back(uniqueMaterials[i]);
        materialToIndex[uniqueMaterials[i]] = static_cast<uint8_t>(i);
    }

    // Step 5: Build per-voxel indices
    uint32_t bitsPerVoxel = (header.GetEncodingType() == BRICK_PALETTE_1BIT) ? 1 :
                           (header.GetEncodingType() == BRICK_PALETTE_2BIT) ? 2 :
                           (header.GetEncodingType() == BRICK_PALETTE_4BIT) ? 4 : 8;

    uint32_t totalBits = 216 * bitsPerVoxel;
    uint32_t totalBytes = (totalBits + 7) / 8;

    size_t indexStartPos = indexDataOut.size();
    indexDataOut.resize(indexStartPos + totalBytes, 0);

    // Pack indices into bit stream
    for (int voxelIdx = 0; voxelIdx < 216; voxelIdx++) {
        uint16_t mat = brick.materials[voxelIdx];
        uint8_t matIndex = (mat == 0) ? 0 : materialToIndex[mat];

        uint32_t bitOffset = voxelIdx * bitsPerVoxel;
        uint32_t byteOffset = bitOffset / 8;
        uint32_t bitInByte = bitOffset % 8;

        // Write bits (may span 2 bytes)
        indexDataOut[indexStartPos + byteOffset] |= (matIndex << bitInByte);
        if (bitInByte + bitsPerVoxel > 8) {
            // Overflow into next byte
            indexDataOut[indexStartPos + byteOffset + 1] |= (matIndex >> (8 - bitInByte));
        }
    }

    return header;
}

//=============================================================================
// Tree Builder Implementation
//=============================================================================

// Helper: build tree recursively, returns node by value (matching reference pattern)
static ContreeNode BuildChunkTree(
    std::vector<ContreeNode>& nodePool,
    std::vector<BrickHeader>& brickPool,
    std::vector<uint16_t>& paletteData,
    std::vector<uint8_t>& indexData,
    const std::function<uint16_t(int, int, int)>& densityFunc,
    int scale,  // log4 of size (6=4096, 4=256, 2=16, 0=1)
    int3 pos);  // Position in world

ContreeBuilder::ContreeBuilder() {
    // Pre-allocate for typical sparse tree
    nodes.reserve(100000);
}

void ContreeBuilder::Build(const std::function<uint16_t(int, int, int)>& densityFunc) {
    nodes.clear();
    bricks.clear();
    brickPaletteData.clear();
    brickIndexData.clear();

    // ========== Create Two Hardcoded Bricks ==========

    // Brick 0: Sphere (radius 2.5)
    std::printf("[Contree] Creating brick 0: sphere\n");
    BrickData sphereBrick;
    float sphereCenter = 2.5f; // Center of 6x6x6 brick
    float sphereRadius = 2.5f;
    int sphereSolidCount = 0;

    for (int bz = 0; bz < 6; bz++) {
        for (int by = 0; by < 6; by++) {
            for (int bx = 0; bx < 6; bx++) {
                int brickIdx = bx + bz * 6 + by * 36; // XZY ordering

                // Test if voxel is inside sphere
                float dx = bx + 0.5f - sphereCenter;
                float dy = by + 0.5f - sphereCenter;
                float dz = bz + 0.5f - sphereCenter;
                float distSq = dx*dx + dy*dy + dz*dz;

                if (distSq <= sphereRadius * sphereRadius) {
                    sphereBrick.materials[brickIdx] = 1; // Material 1
                    sphereSolidCount++;
                } else {
                    sphereBrick.materials[brickIdx] = 0; // Empty
                }
            }
        }
    }

    BrickHeader sphereBrickHeader = BrickBuilder::BuildBrick(sphereBrick, brickPaletteData, brickIndexData);
    bricks.push_back(sphereBrickHeader);

    std::printf("[Contree] Brick 0 (sphere): %d/216 voxels solid, encoding=%d\n",
               sphereSolidCount, sphereBrickHeader.GetEncodingType());

    // Brick 1: 4x4x4 Cube centered in the 6x6x6 brick
    std::printf("[Contree] Creating brick 1: 4x4x4 cube\n");
    BrickData cubeBrick;
    int cubeSolidCount = 0;

    for (int bz = 0; bz < 6; bz++) {
        for (int by = 0; by < 6; by++) {
            for (int bx = 0; bx < 6; bx++) {
                int brickIdx = bx + bz * 6 + by * 36; // XZY ordering

                // 4x4x4 cube centered: voxels [1,4] in each dimension
                bool inCube = (bx >= 1 && bx <= 4) &&
                              (by >= 1 && by <= 4) &&
                              (bz >= 1 && bz <= 4);

                if (inCube) {
                    cubeBrick.materials[brickIdx] = 2; // Material 2
                    cubeSolidCount++;
                } else {
                    cubeBrick.materials[brickIdx] = 0; // Empty
                }
            }
        }
    }

    BrickHeader cubeBrickHeader = BrickBuilder::BuildBrick(cubeBrick, brickPaletteData, brickIndexData);
    bricks.push_back(cubeBrickHeader);

    std::printf("[Contree] Brick 1 (cube): %d/216 voxels solid, encoding=%d\n",
               cubeSolidCount, cubeBrickHeader.GetEncodingType());

    // Visualize both bricks
    std::printf("[Contree] Brick 0 (sphere) - Middle Z-slice (z=3):\n");
    for (int y = 0; y < 6; y++) {
        std::printf("    ");
        for (int x = 0; x < 6; x++) {
            int idx = x + 3 * 6 + y * 36;
            std::printf("%c", sphereBrick.materials[idx] != 0 ? 'S' : '.');
        }
        std::printf("\n");
    }

    std::printf("[Contree] Brick 1 (cube) - Middle Z-slice (z=3):\n");
    for (int y = 0; y < 6; y++) {
        std::printf("    ");
        for (int x = 0; x < 6; x++) {
            int idx = x + 3 * 6 + y * 36;
            std::printf("%c", cubeBrick.materials[idx] != 0 ? 'C' : '.');
        }
        std::printf("\n");
    }

    // Build tree using scale = log2(grid_size)
    // CRITICAL: Scale must be EVEN and >= 2 (decreases by 2 each level, stops at scale=2 leaf)
    // Reference: GRID_SIZE=16 → scale=4 (2^4=16), leaf at scale=2
    // For 256^3: scale=8 (2^8=256), levels: 8 → 6 → 4 → 2 (leaf) = 4 levels
    // This covers [0,256)^3 in world space
    int3 origin(0, 0, 0);
    int scale = 8; // 2^8 = 256 (must be even!)

    std::printf("[Contree] Building tree with scale=%d (size=2^%d=%d)\n",
               scale, scale, 1 << scale);

    ContreeNode root = BuildChunkTree(nodes, bricks, brickPaletteData, brickIndexData, densityFunc, scale, origin);

    // CRITICAL: When we insert root at beginning, all child pointers shift by +1
    // We need to patch all non-leaf child pointers before inserting root
    // NOTE: We must patch ALL non-leaf nodes with children, even if ChildPtr=0!
    for (auto& node : nodes) {
        if (!node.IsLeaf() && node.GetPopMask() != 0) {
            node.SetChildPtr(node.GetChildPtr() + 1);
        }
    }

    // Now patch root's child pointer too
    if (!root.IsLeaf() && root.GetPopMask() != 0) {
        root.SetChildPtr(root.GetChildPtr() + 1);
    }

    // Add root to node pool at index 0
    nodes.insert(nodes.begin(), root);

    // Shrink to fit
    nodes.shrink_to_fit();

    // Validate tree structure
    int errorCount = 0;
    for (size_t i = 0; i < nodes.size(); i++) {
        const auto& node = nodes[i];

        // Check: Leaf nodes must have ChildPtr = 0
        if (node.IsLeaf() && node.GetChildPtr() != 0) {
            std::printf("[Contree ERROR] Node %zu is a leaf but has ChildPtr=%u (should be 0)\n",
                       i, node.GetChildPtr());
            errorCount++;
        }

        // Check: Non-leaf nodes with children must have valid ChildPtr
        if (!node.IsLeaf() && node.GetPopMask() != 0) {
            uint32_t childPtr = node.GetChildPtr();
            if (childPtr >= nodes.size()) {
                std::printf("[Contree ERROR] Node %zu has invalid ChildPtr=%u (out of bounds, size=%zu)\n",
                           i, childPtr, nodes.size());
                errorCount++;
            }
        }

        // Check: Leaf nodes pointing to root (index 0)
        if (node.IsLeaf() && node.GetChildPtr() == 0 && i == 0) {
            // Root can be a leaf, that's fine
        } else if (node.GetChildPtr() == 0 && !node.IsLeaf() && node.GetPopMask() != 0) {
            std::printf("[Contree ERROR] Node %zu is non-leaf with children but ChildPtr=0\n", i);
            errorCount++;
        }
    }

    if (errorCount > 0) {
        std::printf("[Contree] Validation found %d errors!\n", errorCount);
    } else {
        std::printf("[Contree] Validation passed - no errors found\n");
    }

    // Debug output
    std::printf("[Contree] Root node: PopMask=0x%llx, childPtr=%u, isLeaf=%d\n",
               (unsigned long long)nodes[0].GetPopMask(),
               nodes[0].GetChildPtr(),
               nodes[0].IsLeaf());

    // Brick statistics
    std::printf("[Contree] Built %u nodes, %u bricks\n", GetNodeCount(), GetBrickCount());
    std::printf("[Contree] Palette data: %zu uint16s (%zu bytes)\n",
               brickPaletteData.size(), GetBrickPaletteDataSizeBytes());
    std::printf("[Contree] Index data: %zu bytes\n", GetBrickIndexDataSizeBytes());
    std::printf("[Contree] Total memory: %zu bytes\n",
               GetSizeBytes() + GetBricksSizeBytes() + GetBrickPaletteDataSizeBytes() + GetBrickIndexDataSizeBytes());
}

void ContreeBuilder::BuildFromGrid(const uint16_t* grid, int sizeX, int sizeY, int sizeZ) {
    std::printf("[Contree] Building from grid: %dx%dx%d\n", sizeX, sizeY, sizeZ);

    // Create a lambda that samples from the grid
    auto gridSampler = [grid, sizeX, sizeY, sizeZ](int x, int y, int z) -> uint16_t {
        // Bounds check
        if (x < 0 || x >= sizeX || y < 0 || y >= sizeY || z < 0 || z >= sizeZ) {
            return 0; // Empty outside grid bounds
        }

        // Sample from grid (YXZ ordering: x + y * sizeX + z * sizeX * sizeY)
        int index = x + y * sizeX + z * sizeX * sizeY;
        return grid[index];
    };

    // Use existing Build() implementation with the grid sampler
    Build(gridSampler);
}

// Reference implementation pattern from VoxelRT/src/VoxelRT/Render/RendererTree64.cpp:66
static ContreeNode BuildChunkTree(
    std::vector<ContreeNode>& nodePool,
    std::vector<BrickHeader>& brickPool,
    std::vector<uint16_t>& paletteData,
    std::vector<uint8_t>& indexData,
    const std::function<uint16_t(int, int, int)>& densityFunc,
    int scale,
    int3 pos)
{
    ContreeNode node;
    node.SetLeaf(false);
    node.SetChildPtr(0);
    node.SetPopMask(0);

    // Create leaf (reference line 70: if scale == 2)
    // At this point we're at scale=2, which means a 4x4x4 leaf
    // All leaves will point to the single hardcoded brick (index 0)
    if (scale == 2) {
        node.SetLeaf(true);

        // Check if this leaf should be visible using the density function
        // Sample the center of this 4x4x4 leaf
        int centerX = pos.x + 2;
        int centerY = pos.y + 2;
        int centerZ = pos.z + 2;

        bool isVisible = false;
        if (centerX >= 0 && centerX < 256 && centerY >= 0 && centerY < 256 && centerZ >= 0 && centerZ < 256) {
            uint16_t mat = densityFunc(centerX, centerY, centerZ);
            isVisible = (mat != 0);
        }

        if (isVisible) {
            // Randomly choose between brick 0 (sphere) and brick 1 (cube)
            // Use leaf position as seed for deterministic "random" selection
            // Better hash mixing to avoid patterns with multiples of 4
            uint32_t h = pos.x + pos.y * 374761393 + pos.z * 668265263;
            h = (h ^ (h >> 13)) * 1274126177;
            h = h ^ (h >> 16);
            uint16_t brickChoice = h & 1; // 0 = sphere, 1 = cube

            node.SetBrickPtr(brickChoice);
            node.SetDefaultMaterial(brickChoice + 1); // Material 1 or 2
            node.SetPopMask(0xFFFFFFFFFFFFFFFFULL); // Mark all 64 children as occupied

            static int sphereCount = 0;
            static int cubeCount = 0;
            if (brickChoice == 0) sphereCount++; else cubeCount++;

            if (sphereCount + cubeCount < 10) {
                std::printf("[Contree] Leaf at pos=(%d,%d,%d) -> brick %d (%s)\n",
                           pos.x, pos.y, pos.z, brickChoice, brickChoice == 0 ? "sphere" : "cube");
            }

            if ((sphereCount + cubeCount) % 100 == 0) {
                std::printf("[Contree] %d leaves total: %d spheres, %d cubes\n",
                           sphereCount + cubeCount, sphereCount, cubeCount);
            }
        } else {
            // Empty leaf - no brick pointer
            node.SetBrickPtr(0);
            node.SetDefaultMaterial(0);
            node.SetPopMask(0);
        }

        // CRITICAL: Leaf nodes must have ChildPtr = 0
        node.SetChildPtr(0);

        return node;
    }

    // Descend (reference line 95: scale -= 2, meaning divide size by 4)
    scale -= 2;

    // Each child covers 1/4 of parent's size per axis
    // At scale=6: childStep = 2^6 = 64
    // At scale=4: childStep = 2^4 = 16
    // At scale=2: childStep = 2^2 = 4 (but we sample individual voxels in the leaf)
    int childStep = 1 << scale;

    // Build 64 children
    std::vector<ContreeNode> children;
    children.reserve(64);

    for (int i = 0; i < 64; i++) {
        // Extract 3D child position (reference line 100)
        int cx = (i >> 0) & 3;
        int cy = (i >> 4) & 3;
        int cz = (i >> 2) & 3;

        int3 childPos(
            pos.x + cx * childStep,
            pos.y + cy * childStep,
            pos.z + cz * childStep
        );

        // Recurse
        ContreeNode child = BuildChunkTree(nodePool, brickPool, paletteData, indexData, densityFunc, scale, childPos);

        // Only store non-empty children (reference line 103-106)
        if (child.GetPopMask() != 0) {
            node.SetPopMask(node.GetPopMask() | (1ULL << i));
            children.push_back(child);
        }
    }

    // Store children contiguously (reference line 109-110)
    if (!children.empty()) {
        node.SetChildPtr(static_cast<uint32_t>(nodePool.size()));
        nodePool.insert(nodePool.end(), children.begin(), children.end());

        // Debug: Print info about non-leaf nodes
        static int nonLeafCount = 0;
        if (nonLeafCount < 5) {
            std::printf("[Contree] Non-leaf node at scale=%d: PopMask=0x%llx, childPtr=%u, isLeaf=%d, childCount=%zu\n",
                       scale + 2, // +2 because we already decremented
                       (unsigned long long)node.GetPopMask(),
                       node.GetChildPtr(),
                       node.IsLeaf(),
                       children.size());
            nonLeafCount++;
        }
    }

    return node;
}

// Unused now
uint32_t ContreeBuilder::BuildRecursive(
    int level,
    int3 origin,
    int size,
    const std::function<uint16_t(int, int, int)>& densityFunc)
{
    return 0;
}

} // namespace Contree
