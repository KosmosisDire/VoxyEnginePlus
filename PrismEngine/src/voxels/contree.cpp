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

    std::printf("[Contree] Building tree with simple checkerboard materials\n");

    // ========== Create Single Solid Brick ==========
    // Brick 0: All voxels occupied (0xFFFFFFFFFFFFFFFF)
    Brick solidBrick(0xFFFFFFFFFFFFFFFFULL);
    bricks.push_back(solidBrick);
    std::printf("[Contree] Created brick 0: fully solid (64/64 voxels)\n");

    // ========== Create Simple Palette ==========
    // Palette 0: Two materials [0, 1] for black/white checkerboard
    // Format: material_count(1) + materials(N)
    // For now, uncompressed: just store count=2 followed by [0, 1]
    // TODO: When implementing compression, this will change
    paletteBuffer.push_back(2);  // Material count
    paletteBuffer.push_back(1);  // Material 0 (black)
    paletteBuffer.push_back(2);  // Material 1 (white)
    std::printf("[Contree] Created palette 0: 2 materials [0, 1]\n");

    // ========== Create Single Shared Material Indices ==========
    // Material indices 0: 4x4x4 checkerboard pattern (64 indices)
    // All leaves will share this same pattern
    sharedIndicesPtr = static_cast<uint32_t>(materialIndices.size());
    for (int z = 0; z < 4; z++) {
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                // Checkerboard: alternate based on 3D position
                uint16_t materialIdx = ((x + y + z) % 2 == 0) ? 0 : 1;
                materialIndices.push_back(materialIdx);
            }
        }
    }
    std::printf("[Contree] Created shared material indices: 64 values (checkerboard)\n");

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

    // ========== Validation ==========
    int errorCount = 0;
    for (size_t i = 0; i < nodes.size(); i++) {
        const auto& node = nodes[i];

        // Check: Internal nodes with children must have valid ChildPtr
        if (!node.IsLeaf() && node.GetPopMask() != 0) {
            uint32_t childPtr = node.GetChildPtr();
            if (childPtr >= nodes.size()) {
                std::printf("[Contree ERROR] Node %zu has invalid ChildPtr=%u (out of bounds, size=%zu)\n",
                           i, childPtr, nodes.size());
                errorCount++;
            }
        }
    }

    if (errorCount > 0) {
        std::printf("[Contree] Validation found %d errors!\n", errorCount);
    } else {
        std::printf("[Contree] Validation passed - no errors found\n");
    }

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