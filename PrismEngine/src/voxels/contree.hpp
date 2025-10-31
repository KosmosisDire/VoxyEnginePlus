#pragma once

#include <vector>
#include <cstdint>
#include <functional>

namespace Contree {

//=============================================================================
// Constants
//=============================================================================

constexpr uint32_t CONTREE_LEVELS = 4;
constexpr uint32_t CONTREE_BRANCHING = 4;
constexpr uint32_t CONTREE_CHILDREN = 64;  // 4x4x4
constexpr uint32_t BRICK_SIZE = 4;
constexpr uint32_t BRICK_VOXELS = 64;  // 4x4x4
constexpr uint32_t INVALID_PTR = 0xFFFFFFFF;

//=============================================================================
// Core Node Structure (12 bytes)
//=============================================================================

// Internal/Leaf Node - 12 bytes
// INTERNAL nodes:
//   PackedData[0] = IsLeaf(0) | IsAbsolutePtr(1) | ChildPtr(30)
//   PackedData[1] = PopMask low 32 bits
//   PackedData[2] = PopMask high 32 bits
//
// LEAF nodes:
//   PackedData[0] = IsLeaf(1) | flags(1) | OccupancyPtr(30)
//   PackedData[1] = PalettePtr(32)
//   PackedData[2] = MaterialIndicesPtr(32)
struct ContreeNode {
    uint32_t PackedData[4];  // 16 bytes total (was 3, added padding)

    inline bool IsLeaf() const {
        return (PackedData[0] & 1) != 0;
    }
    inline void SetLeaf(bool value) {
        PackedData[0] = (PackedData[0] & ~1u) | (value ? 1u : 0u);
    }

    inline bool IsAbsolutePtr() const {
        return (PackedData[0] & 2) != 0;
    }
    inline void SetAbsolutePtr(bool value) {
        PackedData[0] = (PackedData[0] & ~2u) | (value ? 2u : 0u);
    }

    // Internal node accessors
    inline uint32_t GetChildPtr() const {
        return PackedData[0] >> 2;
    }
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

    // Leaf node accessors (reuse same PackedData fields)
    inline uint32_t GetOccupancyPtr() const {
        return PackedData[0] >> 2;
    }
    inline void SetOccupancyPtr(uint32_t ptr) {
        PackedData[0] = (PackedData[0] & 3u) | (ptr << 2);
    }

    inline uint32_t GetPalettePtr() const {
        return PackedData[1];
    }
    inline void SetPalettePtr(uint32_t ptr) {
        PackedData[1] = ptr;
    }

    inline uint32_t GetMaterialIndicesPtr() const {
        return PackedData[2];
    }
    inline void SetMaterialIndicesPtr(uint32_t ptr) {
        PackedData[2] = ptr;
    }

    ContreeNode() : PackedData{0, 0, 0, 0} {}
};

static_assert(sizeof(ContreeNode) == 16, "ContreeNode must be 16 bytes (GPU alignment)");

//=============================================================================
// Brick Structure (8 bytes) - occupancy only
//=============================================================================

struct Brick {
    uint64_t occupancy;  // 4x4x4 = 64 bits, 1 bit per voxel

    inline bool TestOccupancy(uint32_t idx) const {
        return (occupancy & (1ull << idx)) != 0;
    }

    inline void SetOccupancy(uint32_t idx, bool value) {
        if (value) {
            occupancy |= (1ull << idx);
        } else {
            occupancy &= ~(1ull << idx);
        }
    }

    // Get linear index from 3D position
    static inline uint32_t VoxelIndex(int x, int y, int z) {
        return x + y * 4 + z * 16;
    }

    Brick() : occupancy(0) {}
    explicit Brick(uint64_t occ) : occupancy(occ) {}
};

static_assert(sizeof(Brick) == 8, "Brick must be 8 bytes");

//=============================================================================
// Helper Structures
//=============================================================================

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
};

//=============================================================================
// Contree Builder
//=============================================================================

class ContreeBuilder {
private:
    std::vector<ContreeNode> nodes;
    std::vector<Brick> bricks;              // Occupancy only (shared/deduplicated)
    std::vector<uint16_t> paletteBuffer;    // Material palettes (shared/deduplicated)
    std::vector<uint16_t> materialIndices;  // Per-leaf material data (unique per leaf)

    uint32_t sharedIndicesPtr = 0;          // Pointer to shared material indices for all leaves

    // Recursive tree builder (follows exact algorithm from reference)
    ContreeNode BuildChunkTree(
        const std::function<uint16_t(int, int, int)>& densityFunc,
        int scale,
        int3 pos);

    inline uint32_t ChildIndex(int x, int y, int z) const {
        return x + y * 4 + z * 16;
    }

public:
    ContreeBuilder() = default;

    // Build tree from density function
    // densityFunc(x, y, z) returns materialId (0 = empty, >0 = solid)
    // Covers region [0, 256)^3 in world space
    void Build(const std::function<uint16_t(int, int, int)>& densityFunc);

    // TODO: Add BuildFromGrid when needed
    // TODO: Add BuildWithPalette when needed

    // Get data for GPU upload
    const std::vector<ContreeNode>& GetNodes() const { return nodes; }
    const std::vector<Brick>& GetBricks() const { return bricks; }
    const std::vector<uint16_t>& GetPaletteBuffer() const { return paletteBuffer; }
    const std::vector<uint16_t>& GetMaterialIndices() const { return materialIndices; }

    // Size queries
    uint32_t GetNodeCount() const { return static_cast<uint32_t>(nodes.size()); }
    uint32_t GetBrickCount() const { return static_cast<uint32_t>(bricks.size()); }
    
    size_t GetNodesSizeBytes() const { return nodes.size() * sizeof(ContreeNode); }
    size_t GetBricksSizeBytes() const { return bricks.size() * sizeof(Brick); }
    size_t GetPaletteBufferSizeBytes() const { return paletteBuffer.size() * sizeof(uint16_t); }
    size_t GetMaterialIndicesSizeBytes() const { return materialIndices.size() * sizeof(uint16_t); }
    
    size_t GetTotalSizeBytes() const {
        return GetNodesSizeBytes() + GetBricksSizeBytes() + 
               GetPaletteBufferSizeBytes() + GetMaterialIndicesSizeBytes();
    }

    void Clear() {
        nodes.clear();
        bricks.clear();
        paletteBuffer.clear();
        materialIndices.clear();
    }
};

//=============================================================================
// CPU Helper Functions
//=============================================================================

namespace Helpers {
    inline uint32_t prefix_popcnt64(uint64_t mask, uint32_t width) {
        uint32_t himask = uint32_t(mask);
        uint32_t count = 0;

        if (width >= 32) {
            #ifdef _MSC_VER
                count = __popcnt(himask);
            #else
                count = __builtin_popcount(himask);
            #endif
            himask = uint32_t(mask >> 32);
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