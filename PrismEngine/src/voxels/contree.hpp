#pragma once

#include <vector>
#include <cstdint>
#include <functional>

namespace Contree {

// Constants
constexpr uint32_t CONTREE_LEVELS = 4;
constexpr uint32_t CONTREE_BRANCHING = 4;
constexpr uint32_t CONTREE_CHILDREN = 64; // 4x4x4
constexpr uint32_t INVALID_PTR = 0xFFFFFFFF;

// Flags
constexpr uint8_t FLAG_LEAF = 0x01;
constexpr uint8_t FLAG_DIRTY = 0x02;
constexpr uint8_t FLAG_FULL = 0x04; // All children exist

// Node structure - 16 bytes (12 bytes used + 4 bytes padding)
// Matches VoxelRT layout in first 12 bytes:
// Bytes 0-3:   PackedData[0] = IsLeaf(1) | IsAbsolutePtr(1) | ChildPtr(30)
// Bytes 4-7:   PackedData[1] = PopMask low 32 bits
// Bytes 8-11:  PackedData[2] = PopMask high 32 bits
// Bytes 12-15: Padding (unused)
struct ContreeNode {
    uint32_t PackedData[3];  // First 12 bytes match VoxelRT exactly
    uint32_t _padding;       // Bytes 12-15: Explicit padding

    // Accessor methods matching VoxelRT
    inline bool IsLeaf() const { return (PackedData[0] & 1) != 0; }
    inline void SetLeaf(bool value) {
        PackedData[0] = (PackedData[0] & ~1u) | (value ? 1u : 0u);
    }

    inline bool IsAbsolutePtr() const { return (PackedData[0] & 2) != 0; }
    inline void SetAbsolutePtr(bool value) {
        PackedData[0] = (PackedData[0] & ~2u) | (value ? 2u : 0u);
    }

    inline uint32_t GetChildPtr() const { return PackedData[0] >> 2; }
    inline void SetChildPtr(uint32_t ptr) {
        PackedData[0] = (PackedData[0] & 3u) | (ptr << 2);
    }

    inline uint64_t GetPopMask() const {
        return PackedData[1] | (uint64_t(PackedData[2]) << 32);
    }
    inline void SetPopMask(uint64_t mask) {
        PackedData[1] = uint32_t(mask);
        PackedData[2] = uint32_t(mask >> 32);
    }

    ContreeNode() : PackedData{0, 0, 0}, _padding(0) {}
};

static_assert(sizeof(ContreeNode) == 16, "ContreeNode must be exactly 16 bytes");

// Helper structure for 3D integer coordinates
struct int3 {
    int x, y, z;

    int3() : x(0), y(0), z(0) {}
    int3(int x, int y, int z) : x(x), y(y), z(z) {}

    int3 operator+(const int3& other) const {
        return int3(x + other.x, y + other.y, z + other.z);
    }

    int3 operator*(int scalar) const {
        return int3(x * scalar, y * scalar, z * scalar);
    }

    int3 operator/(int scalar) const {
        return int3(x / scalar, y / scalar, z / scalar);
    }
};

// Custom allocator that builds tree directly into flat array
class ContreeBuilder {
private:
    std::vector<ContreeNode> nodes;

    // Helper: get child index from 3D local position (0-3 in each axis)
    inline uint32_t ChildIndex(int x, int y, int z) const {
        return x + y * 4 + z * 16;
    }

    // Allocate a contiguous block of children
    uint32_t AllocateChildBlock() {
        uint32_t ptr = static_cast<uint32_t>(nodes.size());
        // Pre-allocate 64 child slots (some may be unused for sparse regions)
        nodes.resize(nodes.size() + CONTREE_CHILDREN);
        return ptr;
    }

    // Recursive tree builder
    uint32_t BuildRecursive(
        int level,
        int3 origin,
        int size,
        const std::function<uint16_t(int, int, int)>& densityFunc
    );

    // Calculate voxel size at a given level
    inline int GetVoxelSizeAtLevel(int level) const {
        // Root (level 0): 1024 / 4 = 256 voxels per child
        // Level 1: 256 / 4 = 64 voxels per child
        // Level 2: 64 / 4 = 16 voxels per child
        // Level 3 (leaf): 16 / 4 = 4 voxels per child, then 4^3 voxels in leaf
        int divisions = 1 << (CONTREE_LEVELS - level); // 2^(4-level)
        return 1024 / divisions;
    }

public:
    ContreeBuilder();

    // Build tree from density function
    // densityFunc(x, y, z) returns materialId (0 = empty, >0 = solid)
    // Covers region [0, 0, 0] to [512, 512, 512]
    void Build(const std::function<uint16_t(int, int, int)>& densityFunc);

    // Get flat node array for GPU upload
    const std::vector<ContreeNode>& GetNodes() const { return nodes; }
    uint32_t GetNodeCount() const { return static_cast<uint32_t>(nodes.size()); }
    size_t GetSizeBytes() const { return nodes.size() * sizeof(ContreeNode); }

    // Clear the tree
    void Clear() { nodes.clear(); }
};

// CPU-side helper functions (matching VoxelRT's prefix_popcnt64)
namespace Helpers {
    // Count set bits before position (popcount with width)
    inline uint32_t prefix_popcnt64(uint64_t mask, uint32_t width) {
        uint32_t himask = uint32_t(mask);  // Low 32 bits
        uint32_t count = 0;

        if (width >= 32) {
            #ifdef _MSC_VER
                count = __popcnt(himask);
            #else
                count = __builtin_popcount(himask);
            #endif
            himask = uint32_t(mask >> 32);  // High 32 bits
        }
        uint32_t m = 1u << (width & 31u);
        #ifdef _MSC_VER
            count += __popcnt(himask & (m - 1u));
        #else
            count += __builtin_popcount(himask & (m - 1u));
        #endif

        return count;
    }
}

} // namespace Contree
