#include "contree.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
#include <bitset>

namespace Contree {

// Helper: build tree recursively, returns node by value (matching reference pattern)
static ContreeNode BuildChunkTree(
    std::vector<ContreeNode>& nodePool,
    const std::function<uint16_t(int, int, int)>& densityFunc,
    int scale,  // log4 of size (6=4096, 4=256, 2=16, 0=1)
    int3 pos);  // Position in world

ContreeBuilder::ContreeBuilder() {
    // Pre-allocate for typical sparse tree
    nodes.reserve(100000);
}

void ContreeBuilder::Build(const std::function<uint16_t(int, int, int)>& densityFunc) {
    nodes.clear();

    // Build tree using scale = log2(grid_size)
    // CRITICAL: Scale must be EVEN and >= 2 (decreases by 2 each level, stops at scale=2 leaf)
    // Reference: GRID_SIZE=16 → scale=4 (2^4=16), leaf at scale=2
    // For 256^3: scale=8 (2^8=256), levels: 8 → 6 → 4 → 2 (leaf) = 4 levels
    // This covers [0,256)^3 in world space
    int3 origin(0, 0, 0);
    int scale = 8; // 2^8 = 256 (must be even!)

    std::printf("[Contree] Building tree with scale=%d (size=2^%d=%d)\n",
               scale, scale, 1 << scale);

    ContreeNode root = BuildChunkTree(nodes, densityFunc, scale, origin);

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
}

// Reference implementation pattern from VoxelRT/src/VoxelRT/Render/RendererTree64.cpp:66
static ContreeNode BuildChunkTree(
    std::vector<ContreeNode>& nodePool,
    const std::function<uint16_t(int, int, int)>& densityFunc,
    int scale,
    int3 pos)
{
    ContreeNode node;
    node.SetLeaf(false);
    node.SetChildPtr(0);
    node.SetPopMask(0);

    // Create leaf (reference line 70: if scale == 2)
    // At this point we're at scale=2, which means a 4x4x4 voxel leaf
    if (scale == 2) {
        node.SetLeaf(true);

        // Sample 4x4x4 voxels
        uint64_t occupancy = 0;

        static int sampleCount = 0;
        static int solidCount = 0;
        static bool debugPrinted = false;

        for (int i = 0; i < 64; i++) {
            int lx = (i >> 0) & 3;
            int ly = (i >> 4) & 3;
            int lz = (i >> 2) & 3;

            int wx = pos.x + lx;
            int wy = pos.y + ly;
            int wz = pos.z + lz;

            // Debug first few samples
            if (!debugPrinted && sampleCount < 10) {
                std::printf("[Contree] Sample %d: pos=(%d,%d,%d) local=(%d,%d,%d) world=(%d,%d,%d)\n",
                           sampleCount, pos.x, pos.y, pos.z, lx, ly, lz, wx, wy, wz);
            }

            // Only sample if within valid region [0, 256)
            if (wx >= 0 && wx < 256 && wy >= 0 && wy < 256 && wz >= 0 && wz < 256) {
                uint16_t mat = densityFunc(wx, wy, wz);
                sampleCount++;
                if (mat != 0) {
                    solidCount++;
                    occupancy |= (1ULL << i);
                    if (solidCount <= 5) {
                        float dx = wx - 128.0f;
                        float dy = wy - 128.0f;
                        float dz = wz - 128.0f;
                        float dist = std::sqrt(dx*dx + dy*dy + dz*dz);
                        std::printf("[Contree] Solid voxel %d: world=(%d,%d,%d) leafPos=(%d,%d,%d) localIdx=%d dist=%.1f\n",
                                   solidCount, wx, wy, wz, pos.x, pos.y, pos.z, i, dist);
                    }
                }
            }
        }

        if (sampleCount == 10) {
            debugPrinted = true;
            std::printf("[Contree] Tested sphere at (128,128,128): %d\n",
                       densityFunc(128, 128, 128));
            std::printf("[Contree] Tested sphere at (128,128,178): %d\n",
                       densityFunc(128, 128, 178));
        }

        if (sampleCount % 100000 == 0) {
            std::printf("[Contree] Sampled %d voxels, %d solid\n", sampleCount, solidCount);
        }

        node.SetPopMask(occupancy);

        // CRITICAL: Leaf nodes must have ChildPtr = 0 to avoid pointing to random nodes
        node.SetChildPtr(0);

        // Debug
        if (occupancy != 0) {
            static int leafsWithVoxels = 0;
            static int totalVoxels = 0;
            int voxelsInLeaf = static_cast<int>(std::bitset<64>(occupancy).count());
            totalVoxels += voxelsInLeaf;
            leafsWithVoxels++;

            if (leafsWithVoxels == 1) {
                std::printf("[Contree] First leaf with voxels at pos=(%d,%d,%d), count=%d\n",
                           pos.x, pos.y, pos.z, voxelsInLeaf);
            }
            if (leafsWithVoxels % 100 == 0) {
                std::printf("[Contree] %d leaves with voxels, %d total voxels\n",
                           leafsWithVoxels, totalVoxels);
            }
        }

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
        ContreeNode child = BuildChunkTree(nodePool, densityFunc, scale, childPos);

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
