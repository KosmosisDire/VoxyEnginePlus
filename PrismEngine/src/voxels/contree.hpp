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

// Node structure - 16 bytes
// Bytes 0-3:   PackedData[0] = IsLeaf(1) | IsAbsolutePtr(1) | ChildPtr(30)
// Bytes 4-7:   PackedData[1] = PopMask low 32 bits
// Bytes 8-11:  PackedData[2] = PopMask high 32 bits
// Bytes 12-15: PackedData[3] = BrickPtr(16) | DefaultMaterial(16) [for leaves only]
struct ContreeNode {
    uint32_t PackedData[4];  // 16 bytes total

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

    // For leaf nodes: brick pointer and default material
    inline uint16_t GetBrickPtr() const { return uint16_t(PackedData[3] & 0xFFFF); }
    inline void SetBrickPtr(uint16_t ptr) {
        PackedData[3] = (PackedData[3] & 0xFFFF0000) | ptr;
    }

    inline uint16_t GetDefaultMaterial() const { return uint16_t(PackedData[3] >> 16); }
    inline void SetDefaultMaterial(uint16_t matId) {
        PackedData[3] = (PackedData[3] & 0xFFFF) | (uint32_t(matId) << 16);
    }

    ContreeNode() : PackedData{0, 0, 0, 0} {}
};

static_assert(sizeof(ContreeNode) == 16, "ContreeNode must be exactly 16 bytes");

//=============================================================================
// Brick Structures (6x6x6 voxel bricks with palette compression)
//=============================================================================

// Brick encoding types
enum BrickEncoding : uint8_t {
    BRICK_SINGLE_MATERIAL = 0,    // materialCount=1, no per-voxel data needed
    BRICK_PALETTE_1BIT = 1,       // 2 materials, 1 bit per voxel
    BRICK_PALETTE_2BIT = 2,       // 3-4 materials, 2 bits per voxel
    BRICK_PALETTE_4BIT = 3,       // 5-16 materials, 4 bits per voxel
    BRICK_PALETTE_8BIT = 4,       // 17-256 materials, 8 bits per voxel
};

// BrickHeader: 32 bytes (8 x uint32)
// Word 0-6: occupancy (216 bits) + materialCount (8 bits)
//   - Bits [0-215]: occupancy data
//   - Bits [216-223]: materialCount (8 bits)
// Word 7: packed metadata
//   - Bits [0-7]: encodingType
//   - Bits [8-15]: flags
//   - Bits [16-31]: palettePtr
struct BrickHeader {
    uint32_t data[8];  // 32 bytes total

    // Accessors for occupancy bits [0-215]
    inline bool TestOccupancy(uint32_t idx) const {
        if (idx >= 216) return false;
        uint32_t wordIdx = idx / 32;
        uint32_t bitIdx = idx % 32;
        return (data[wordIdx] & (1u << bitIdx)) != 0;
    }

    inline void SetOccupancy(uint32_t idx, bool value) {
        if (idx >= 216) return;
        uint32_t wordIdx = idx / 32;
        uint32_t bitIdx = idx % 32;
        if (value) {
            data[wordIdx] |= (1u << bitIdx);
        } else {
            data[wordIdx] &= ~(1u << bitIdx);
        }
    }

    // Accessors for materialCount in bits [216-223] (high byte of data[6])
    inline uint8_t GetMaterialCount() const {
        return uint8_t(data[6] >> 24);
    }

    inline void SetMaterialCount(uint8_t count) {
        data[6] = (data[6] & 0x00FFFFFF) | (uint32_t(count) << 24);
    }

    // Accessors for data[7] packed fields
    inline uint8_t GetEncodingType() const {
        return uint8_t(data[7] & 0xFF);
    }

    inline void SetEncodingType(uint8_t type) {
        data[7] = (data[7] & 0xFFFFFF00) | type;
    }

    inline uint8_t GetFlags() const {
        return uint8_t((data[7] >> 8) & 0xFF);
    }

    inline void SetFlags(uint8_t f) {
        data[7] = (data[7] & 0xFFFF00FF) | (uint32_t(f) << 8);
    }

    inline uint16_t GetPalettePtr() const {
        return uint16_t(data[7] >> 16);
    }

    inline void SetPalettePtr(uint16_t ptr) {
        data[7] = (data[7] & 0x0000FFFF) | (uint32_t(ptr) << 16);
    }

    BrickHeader() : data{0, 0, 0, 0, 0, 0, 0, 0} {}
};

static_assert(sizeof(BrickHeader) == 32, "BrickHeader must be exactly 32 bytes");

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

// Brick data: represents a 6x6x6 voxel volume with materials
struct BrickData {
    uint16_t materials[216];  // Material ID per voxel (0 = empty)

    BrickData() {
        std::fill(std::begin(materials), std::end(materials), uint16_t(0));
    }
};

// Brick builder: analyzes a brick and generates compressed representation
class BrickBuilder {
public:
    // Analyze brick and determine optimal encoding
    static BrickHeader BuildBrick(
        const BrickData& brick,
        std::vector<uint16_t>& paletteDataOut,
        std::vector<uint8_t>& indexDataOut
    );

    // Helper: count unique materials in brick
    static uint8_t CountUniqueMaterials(const BrickData& brick, std::vector<uint16_t>& uniqueMaterials);
};

// Custom allocator that builds tree directly into flat array
class ContreeBuilder {
private:
    std::vector<ContreeNode> nodes;
    std::vector<BrickHeader> bricks;
    std::vector<uint16_t> brickPaletteData;  // Variable-size palette entries
    std::vector<uint8_t> brickIndexData;     // Variable-size per-voxel indices

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

    // Get brick data for GPU upload
    const std::vector<BrickHeader>& GetBricks() const { return bricks; }
    uint32_t GetBrickCount() const { return static_cast<uint32_t>(bricks.size()); }
    size_t GetBricksSizeBytes() const { return bricks.size() * sizeof(BrickHeader); }

    const std::vector<uint16_t>& GetBrickPaletteData() const { return brickPaletteData; }
    size_t GetBrickPaletteDataSizeBytes() const { return brickPaletteData.size() * sizeof(uint16_t); }

    const std::vector<uint8_t>& GetBrickIndexData() const { return brickIndexData; }
    size_t GetBrickIndexDataSizeBytes() const { return brickIndexData.size(); }

    // Clear the tree
    void Clear() {
        nodes.clear();
        bricks.clear();
        brickPaletteData.clear();
        brickIndexData.clear();
    }
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
