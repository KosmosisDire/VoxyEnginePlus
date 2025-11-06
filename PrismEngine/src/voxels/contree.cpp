#include "contree.hpp"
#include <cstdio>

namespace Contree {

//=============================================================================
// Builder Implementation
//=============================================================================

void ContreeBuilder::Build(const std::function<uint16_t(int, int, int)>& densityFunc) {
    nodes.clear();
    bricks.clear();
    paletteBuffer.clear();
    materialIndices.clear();

    std::printf("[Contree] Building tree with procedural density-based occupancy (%dx%dx%d bricks)\n",
               BRICK_SIZE, BRICK_SIZE, BRICK_SIZE);

    // ========== Create Single Procedural Brick ==========
    // Brick 0: Build occupancy from density function (BRICK_SIZE^3 = BRICK_VOXELS bits)
    Brick proceduralBrick;
    uint32_t occupiedCount = 0;

    // Sample density function at origin for this brick pattern
    float centerX = BRICK_SIZE / 2.0f;
    float centerY = BRICK_SIZE / 2.0f;
    float centerZ = BRICK_SIZE / 2.0f;
    float radius = BRICK_SIZE / 2.0f - 0.5f;  // Slightly smaller than brick

    for (int z = 0; z < BRICK_SIZE; z++) {
    for (int y = 0; y < BRICK_SIZE; y++) {
    for (int x = 0; x < BRICK_SIZE; x++) {
        // Create a sphere in the center of the brick
        float dx = x + 0.5f - centerX;
        float dy = y + 0.5f - centerY;
        float dz = z + 0.5f - centerZ;
        float dist = sqrtf(dx*dx + dy*dy + dz*dz);
        bool isOccupied = (dist <= radius) || y <= 5;

        // Set occupancy bit
        uint32_t voxelIdx = Brick::VoxelIndex(x, y, z);
        proceduralBrick.SetOccupancy(voxelIdx, isOccupied);
        if (isOccupied) occupiedCount++;
    }}}

    bricks.push_back(proceduralBrick);
    std::printf("[Contree] Created brick 0: procedural (%u/%u voxels occupied, %dx%dx%d)\n",
               occupiedCount, BRICK_VOXELS, BRICK_SIZE, BRICK_SIZE, BRICK_SIZE);

    // ========== Create Position-Based Palette ==========
    // Palette 0: Generate unique materials based on 3D position
    // For a BRICK_SIZE^3 brick, we can encode position directly or use a mapping
    // Here we'll create materials that map to RGB based on normalized position
    // Format: material_count(1) + materials(N)
    constexpr int numMaterials = BRICK_SIZE * BRICK_SIZE * BRICK_SIZE;  // One material per coordinate value (0 to BRICK_SIZE-1)
    paletteBuffer.push_back(numMaterials);  // Material count
    for (int i = 0; i < numMaterials; i++) {
        paletteBuffer.push_back(i + 1);  // Materials 1 to BRICK_SIZE
    }
    std::printf("[Contree] Created palette 0: %d materials [1-%d]\n", numMaterials, numMaterials);

    // ========== Create BRICK_SIZE^3 Brick Material Indices ==========
    // Material indices: BRICK_SIZE^3 = BRICK_VOXELS values
    // Generate material based on 3D position within the brick
    sharedIndicesPtr = static_cast<uint32_t>(materialIndices.size());
    for (int x = 0; x < BRICK_SIZE; x++) {
        for (int y = 0; y < BRICK_SIZE; y++) {
            for (int z = 0; z < BRICK_SIZE; z++) {
                uint16_t materialIdx = x + y * BRICK_SIZE + z * BRICK_SIZE * BRICK_SIZE;
                materialIndices.push_back(materialIdx);
            }
        }
    }
    std::printf("[Contree] Created shared material indices: %u values (%dx%dx%d position-based)\n",
               BRICK_VOXELS, BRICK_SIZE, BRICK_SIZE, BRICK_SIZE);

    // ========== Build Tree ==========
    // Scale = 8 means 2^8 = 256 voxels per side
    // Covers [0, 256)^3 in world space
    // Must be even and >= 2, decreases by 2 each level
    int3 origin(0, 0, 0);
    int scale = 8;

    std::printf("[Contree] Building tree with scale=%d (size=2^%d=%d)\n",
               scale, scale, 1 << scale);

    ContreeNode root = BuildChunkTree(densityFunc, scale, origin);

    // ========== Patch Child Pointers ==========
    // CRITICAL: When we insert root at beginning, all child pointers shift by +1
    // Must patch ALL non-leaf nodes with children before inserting root
    for (auto& node : nodes) {
        if (!node.IsLeaf() && node.GetPopMask() != 0) {
            node.SetChildPtr(node.GetChildPtr() + 1);
        }
    }

    // Patch root's child pointer too
    if (!root.IsLeaf() && root.GetPopMask() != 0) {
        root.SetChildPtr(root.GetChildPtr() + 1);
    }

    // Add root at index 0
    nodes.insert(nodes.begin(), root);
    nodes.shrink_to_fit();

    // ========== Debug Output ==========
    std::printf("[Contree] Root node: PopMask=0x%llx, childPtr=%u, isLeaf=%d\n",
               (unsigned long long)nodes[0].GetPopMask(),
               nodes[0].GetChildPtr(),
               nodes[0].IsLeaf());

    std::printf("[Contree] Built %u nodes, %u bricks\n", GetNodeCount(), GetBrickCount());
    std::printf("[Contree] Palette data: %zu uint16s (%zu bytes)\n",
               paletteBuffer.size(), GetPaletteBufferSizeBytes());
    std::printf("[Contree] Material indices: %zu uint16s (%zu bytes)\n",
               materialIndices.size(), GetMaterialIndicesSizeBytes());
    std::printf("[Contree] Total memory: %zu bytes\n", GetTotalSizeBytes());
}

ContreeNode ContreeBuilder::BuildChunkTree(
    const std::function<uint16_t(int, int, int)>& densityFunc,
    int scale,
    int3 pos)
{
    ContreeNode node;
    node.SetLeaf(false);
    node.SetChildPtr(0);
    node.SetPopMask(0);

    // ========== Create Leaf ==========
    // At scale=2, we create a leaf representing a 4x4x4 region
    if (scale == 2) {
        node.SetLeaf(true);

        // Sample center of this 4x4x4 leaf to determine if visible
        int centerX = pos.x + 2;
        int centerY = pos.y + 2;
        int centerZ = pos.z + 2;

        bool isVisible = false;
        if (centerX >= 0 && centerX < 256 && 
            centerY >= 0 && centerY < 256 && 
            centerZ >= 0 && centerZ < 256) {
            uint16_t mat = densityFunc(centerX, centerY, centerZ);
            isVisible = (mat != 0);
        }

        if (isVisible) {
            // ========== Setup Leaf Pointers (all point to shared data) ==========
            // Occupancy: All leaves point to brick 0 (fully solid)
            node.SetOccupancyPtr(0);

            // Palette: All leaves point to palette 0 (materials [0, 1])
            node.SetPalettePtr(0);

            // Material indices: All leaves point to the shared checkerboard pattern (indices 0-63)
            node.SetMaterialIndicesPtr(sharedIndicesPtr);

            // CRITICAL: For leaf nodes, GetPopMask() reads PackedData[1-2] which are now pointers (both 0)
            // So GetPopMask() returns 0, making the parent think this leaf is empty!
            // Solution: Use PackedData[3] (padding) as a "non-empty" flag
            node.PackedData[3] = 1;  // Mark as non-empty so parent includes it
        } else {
            // Empty leaf - all pointers = 0, padding = 0
            node.SetOccupancyPtr(0);
            node.SetPalettePtr(0);
            node.SetMaterialIndicesPtr(0);
            node.PackedData[3] = 0;  // Mark as empty
        }

        return node;
    }

    // ========== Descend to Children ==========
    // Decrease scale by 2 (divide size by 4)
    scale -= 2;
    int childStep = 1 << scale;  // Size of each child

    // Build 64 children (4x4x4)
    std::vector<ContreeNode> children;
    children.reserve(64);

    for (int i = 0; i < 64; i++) {
        // Extract 3D child position (CRITICAL: exact ordering from reference)
        int cx = (i >> 0) & 3;  // X from bits 0-1
        int cy = (i >> 4) & 3;  // Y from bits 4-5
        int cz = (i >> 2) & 3;  // Z from bits 2-3

        int3 childPos(
            pos.x + cx * childStep,
            pos.y + cy * childStep,
            pos.z + cz * childStep
        );

        // Recurse
        ContreeNode child = BuildChunkTree(densityFunc, scale, childPos);

        // Only store non-empty children
        // For leaves, GetPopMask() might be 0 even if visible (pointers are 0)
        // Check PackedData[3] flag instead (set to 1 for visible leaves)
        bool childNonEmpty = child.IsLeaf() ? (child.PackedData[3] != 0) : (child.GetPopMask() != 0);

        if (childNonEmpty) {
            node.SetPopMask(node.GetPopMask() | (1ULL << i));
            children.push_back(child);
        }
    }

    // Store children contiguously
    if (!children.empty()) {
        node.SetChildPtr(static_cast<uint32_t>(nodes.size()));
        nodes.insert(nodes.end(), children.begin(), children.end());
    }

    return node;
}

} // namespace Contree