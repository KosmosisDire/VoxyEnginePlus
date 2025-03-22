#pragma once

#include <vector>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <stdexcept>
#include "tree-shapes.hpp"

// Forward declarations
class DenseGrid;
class Tree64;

// Main node structure for SVO - memory layout must match GPU expectations exactly
struct RawNode
{
    uint32_t PackedData[3];

    RawNode()
    {
        PackedData[0] = 0;
        PackedData[1] = 0;
        PackedData[2] = 0;
    }

    // Accessors and mutators - exact bit layout is critical for GPU compatibility
    bool GetIsLeaf() const
    {
        return (PackedData[0] & 1) != 0;
    }

    uint32_t GetChildPtr() const
    {
        return PackedData[0] >> 1;
    }

    uint64_t GetPopMask() const
    {
        return static_cast<uint64_t>(PackedData[1]) |
               (static_cast<uint64_t>(PackedData[2]) << 32);
    }

    void SetIsLeaf(bool isLeaf)
    {
        if (isLeaf)
        {
            PackedData[0] |= 1;
        }
        else
        {
            PackedData[0] &= ~1u;
        }
    }

    void SetChildPtr(uint32_t childPtr)
    {
        PackedData[0] &= 1;  // Keep only IsLeaf bit
        PackedData[0] |= (childPtr << 1);
    }

    void SetPopMask(uint64_t popMask)
    {
        PackedData[1] = popMask & 0xFFFFFFFF;
        PackedData[2] = (popMask >> 32) & 0xFFFFFFFF;
    }
};

// Result structure containing the complete octree data
struct OctreeData
{
    std::vector<RawNode> nodes;
    std::vector<uint8_t> leafData;
};

// Helper functions for bit operations (exact match with GPU code)
inline uint32_t popcnt_var64(uint64_t mask, uint32_t pos)
{
    if (pos == 0)
    {
        return 0;
    }

    uint64_t bitMask = (1ULL << pos) - 1ULL;

#if defined(__GNUC__) || defined(__clang__)
    return __builtin_popcountll(mask & bitMask);
#else
    // Platform-independent fallback
    uint64_t bits = mask & bitMask;
    uint32_t count = 0;

    while (bits)
    {
        count += bits & 1;
        bits >>= 1;
    }

    return count;
#endif
}

inline uint32_t popcount(uint64_t value)
{
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_popcountll(value);
#else
    // Platform-independent fallback
    uint32_t count = 0;

    while (value)
    {
        count += value & 1;
        value >>= 1;
    }

    return count;
#endif
}

// Left-pack operation (exact match with GPU expectation)
inline void LeftPack(uint8_t data[64], uint64_t mask)
{
    uint8_t temp[64];
    std::memcpy(temp, data, 64);

    uint32_t j = 0;

    for (uint32_t i = 0; i < 64; i++)
    {
        if ((mask >> i) & 1)
        {
            data[j++] = temp[i];
        }
    }
}

// Get cell index from position (must match shader code exactly)
inline int GetNodeCellIndex(const Vec3& pos, int scale_exp)
{
    Vec3 cellPos = (pos >> scale_exp) & 3;
    return cellPos.x + (cellPos.z * 4) + (cellPos.y * 16);
}

/**
 * @class DenseGrid
 * @brief Efficient representation of a 3D voxel grid
 *
 * This class stores voxel data in a packed bit format (8 voxels per byte)
 * to minimize memory usage while providing fast access.
 */
class DenseGrid
{
    public:
        /**
         * @brief Constructor for a grid of specified size
         * @param size Size of the grid (must be a power of 2)
         */
        DenseGrid(int size) : size(size)
        {
            assert((size & (size - 1)) == 0 && "Size must be a power of 2");
            // Calculate number of bytes needed (8 voxels per byte)
            size_t numVoxels = static_cast<size_t>(size) * size * size;
            size_t numBytes = (numVoxels + 7) / 8; // Ceiling division by 8
            data.resize(numBytes, 0);
        }

        /**
         * @brief Initialize grid from a shape
         * @param shape The shape to voxelize
         */
        void InitializeFromShape(const Shape& shape)
        {
            // Clear existing data
            std::fill(data.begin(), data.end(), 0);

            // Fill grid with shape
            for (int y = 0; y < size; ++y)
            {
                for (int z = 0; z < size; ++z)
                {
                    for (int x = 0; x < size; ++x)
                    {
                        Vec3 pos(x, y, z);
                        Vec3 center(x + 0.5f, y + 0.5f, z + 0.5f);

                        if (shape.Contains(center))
                        {
                            SetVoxel(pos, true);
                        }
                    }
                }
            }
        }

        /**
         * @brief Get voxel value at position
         * @param pos The 3D position
         * @return true if voxel is filled, false if empty or out of bounds
         */
        bool GetVoxel(const Vec3& pos) const
        {
            if (pos.x < 0 || pos.y < 0 || pos.z < 0 ||
                    pos.x >= size || pos.y >= size || pos.z >= size)
            {
                return false; // Out of bounds
            }

            size_t bitIndex = GetBitIndex(pos);
            size_t byteIndex = bitIndex / 8;
            size_t bitOffset = bitIndex % 8;

            return (data[byteIndex] & (1 << bitOffset)) != 0;
        }

        /**
         * @brief Set voxel value at position
         * @param pos The 3D position
         * @param value true to fill, false to empty
         */
        void SetVoxel(const Vec3& pos, bool value)
        {
            if (pos.x < 0 || pos.y < 0 || pos.z < 0 ||
                    pos.x >= size || pos.y >= size || pos.z >= size)
            {
                return; // Out of bounds
            }

            size_t bitIndex = GetBitIndex(pos);
            size_t byteIndex = bitIndex / 8;
            size_t bitOffset = bitIndex % 8;

            if (value)
            {
                data[byteIndex] |= (1 << bitOffset);
            }
            else
            {
                data[byteIndex] &= ~(1 << bitOffset);
            }
        }

        /**
         * @brief Check if cubic region is completely filled
         * @param pos Origin of the cube
         * @param scale Log2 of the cube size
         * @return true if all voxels in the region are filled
         */
        bool IsRegionFilled(const Vec3& pos, int scale) const
        {
            int cubeSize = 1 << scale;

            if (pos.x < 0 || pos.y < 0 || pos.z < 0 ||
                    pos.x + cubeSize > size || pos.y + cubeSize > size || pos.z + cubeSize > size)
            {
                return false; // Out of bounds
            }

            for (int y = 0; y < cubeSize; ++y)
            {
                for (int z = 0; z < cubeSize; ++z)
                {
                    for (int x = 0; x < cubeSize; ++x)
                    {
                        if (!GetVoxel(Vec3(pos.x + x, pos.y + y, pos.z + z)))
                        {
                            return false;
                        }
                    }
                }
            }

            return true;
        }

        /**
         * @brief Check if cubic region is completely empty
         * @param pos Origin of the cube
         * @param scale Log2 of the cube size
         * @return true if all voxels in the region are empty
         */
        bool IsRegionEmpty(const Vec3& pos, int scale) const
        {
            int cubeSize = 1 << scale;

            if (pos.x < 0 || pos.y < 0 || pos.z < 0 ||
                    pos.x + cubeSize > size || pos.y + cubeSize > size || pos.z + cubeSize > size)
            {
                return true; // Out of bounds is considered empty
            }

            for (int y = 0; y < cubeSize; ++y)
            {
                for (int z = 0; z < cubeSize; ++z)
                {
                    for (int x = 0; x < cubeSize; ++x)
                    {
                        if (GetVoxel(Vec3(pos.x + x, pos.y + y, pos.z + z)))
                        {
                            return false;
                        }
                    }
                }
            }

            return true;
        }

        /**
         * @brief Get size of the grid
         * @return Size of the grid in voxels (power of 2)
         */
        int GetSize() const
        {
            return size;
        }

        /**
         * @brief Get raw data for direct access
         * @return Reference to the internal data array
         */
        const std::vector<uint8_t>& GetData() const
        {
            return data;
        }

        /**
         * @brief Set raw data directly
         * @param newData New data to copy into the grid
         * @throws std::invalid_argument if data size doesn't match
         */
        void SetData(const std::vector<uint8_t>& newData)
        {
            if (newData.size() != data.size())
            {
                throw std::invalid_argument("Data size mismatch");
            }

            data = newData;
        }

    private:
        std::vector<uint8_t> data;  // 8 voxels per byte for compact storage
        int size;  // Size of the grid (must be a power of 2)

        /**
         * @brief Calculate bit index from 3D position
         * @param pos The 3D position
         * @return Flat index into the bit array
         */
        size_t GetBitIndex(const Vec3& pos) const
        {
            // Using Z-order curve for better cache locality
            return static_cast<size_t>(pos.x) +
                   (static_cast<size_t>(pos.z) * size) +
                   (static_cast<size_t>(pos.y) * size * size);
        }
};

/**
 * @class Tree64
 * @brief A 64-tree representation of voxel data optimized for GPU traversal
 *
 * This class manages both a dense grid representation and the resulting sparse tree
 * structure that's compatible with GPU traversal.
 */
class Tree64
{
    public:
        /**
         * @brief Constructor
         * @param maxScale Maximum scale of the tree (log2 of root node size)
         */
        Tree64(int maxScale = 10) : grid(1 << maxScale), maxScale(maxScale)
        {
            // Initialize with empty grid
        }

        /**
         * @brief Initialize tree from a shape
         * @param shape The shape to voxelize and convert to a tree
         */
        void InitializeFromShape(const Shape& shape)
        {
            grid.InitializeFromShape(shape);
            GenerateTree();
        }

        /**
         * @brief Generate tree from current grid data
         *
         * This rebuilds the entire tree structure from the current grid.
         */
        void GenerateTree()
        {
            octreeData.nodes.clear();
            octreeData.leafData.clear();

            // Reserve space for efficiency
            octreeData.nodes.reserve(1000);
            octreeData.leafData.reserve(1000);

            // Ensure space for root node
            octreeData.nodes.resize(1);

            // Generate tree starting at root
            octreeData.nodes[0] = GenerateTreeFromGrid(octreeData.nodes, octreeData.leafData,
                                  maxScale, Vec3(0, 0, 0));
        }

        /**
         * @brief Modify a single voxel and optionally regenerate the tree
         * @param pos Position to modify
         * @param value New voxel value
         * @param regenerate Whether to regenerate the tree immediately
         */
        void ModifyVoxel(const Vec3& pos, bool value, bool regenerate = true)
        {
            grid.SetVoxel(pos, value);

            if (regenerate)
            {
                GenerateTree();
            }
        }

        /**
         * @brief Batch modify multiple voxels with a single tree regeneration
         * @param positions Vector of positions to modify
         * @param values Vector of values for each position
         */
        void BatchModifyVoxels(const std::vector<Vec3>& positions, const std::vector<bool>& values)
        {
            if (positions.size() != values.size())
            {
                throw std::invalid_argument("Position and value arrays must be the same size");
            }

            for (size_t i = 0; i < positions.size(); ++i)
            {
                grid.SetVoxel(positions[i], values[i]);
            }

            // Regenerate the tree once after all modifications
            GenerateTree();
        }

        /**
         * @brief Replace entire grid with a new one and regenerate tree
         * @param newGrid The new grid to use
         */
        void ReplaceGrid(const DenseGrid& newGrid)
        {
            if (newGrid.GetSize() != grid.GetSize())
            {
                throw std::invalid_argument("Grid size mismatch");
            }

            // Directly replace the data
            grid.SetData(newGrid.GetData());

            // Regenerate the tree
            GenerateTree();
        }

        /**
         * @brief Get output data for GPU traversal
         * @return Constant reference to the octree data structure
         */
        const OctreeData& GetOctreeData() const
        {
            return octreeData;
        }

        /**
         * @brief Access the grid directly for advanced operations
         * @return Reference to the internal grid
         */
        DenseGrid& GetGrid()
        {
            return grid;
        }

        /**
         * @brief Get raw node and leaf data vectors for direct GPU upload
         * @param nodes Output parameter for node data
         * @param leafData Output parameter for leaf data
         */
        void GetRawData(std::vector<RawNode>& nodes, std::vector<uint8_t>& leafData) const
        {
            nodes = octreeData.nodes;
            leafData = octreeData.leafData;
        }

    private:
        DenseGrid grid;           // Dense grid representation
        OctreeData octreeData;    // Sparse tree representation
        int maxScale;             // Maximum tree scale

        /**
         * @brief Helper function to generate tree from the dense grid
         * @param nodes Output vector of nodes
         * @param leafData Output vector of leaf data
         * @param scale Current scale being processed
         * @param pos Position of the current node
         * @return The generated RawNode
         */
        RawNode GenerateTreeFromGrid(
            std::vector<RawNode>& nodes,
            std::vector<uint8_t>& leafData,
            int32_t scale,
            const Vec3& pos = Vec3(0, 0, 0)
        )
        {
            // Initialize empty node
            RawNode node;

            // Skip if completely outside or empty
            if (grid.IsRegionEmpty(pos, scale))
            {
                return node;
            }

            // Create leaf node at minimum scale
            if (scale == 2)
            {
                uint8_t temp[64] = {0};

                if (grid.IsRegionFilled(pos, scale))
                {
                    // Fill all voxels
                    std::memset(temp, 255, 64);
                }
                else
                {
                    // Test each individual voxel - using exact indexing from GPU shader
                    for (int y = 0; y < 4; ++y)
                    {
                        for (int z = 0; z < 4; ++z)
                        {
                            for (int x = 0; x < 4; ++x)
                            {
                                // Must match exact formula in shader's GetNodeCellIndex
                                int index = x + (z * 4) + (y * 16);

                                // Test voxel
                                if (grid.GetVoxel(Vec3(pos.x + x, pos.y + y, pos.z + z)))
                                {
                                    temp[index] = 255;
                                }
                            }
                        }
                    }
                }

                // Create population mask - 1 bit per occupied voxel
                uint64_t popMask = 0;

                for (int i = 0; i < 64; ++i)
                {
                    if (temp[i] != 0)
                    {
                        popMask |= 1ULL << i;
                    }
                }

                if (popMask != 0)
                {
                    // Create leaf node
                    node.SetIsLeaf(true);
                    node.SetPopMask(popMask);

                    // Store leaf data - only store non-empty voxels
                    uint8_t packedTemp[64];
                    std::memcpy(packedTemp, temp, 64);

                    // Critical: Pack the data exactly as the shader expects
                    uint32_t leafPtr = static_cast<uint32_t>(leafData.size());
                    node.SetChildPtr(leafPtr);

                    // Use LeftPack to arrange leaf data in the expected order
                    LeftPack(packedTemp, popMask);
                    uint32_t setCount = popcount(popMask);
                    leafData.insert(leafData.end(), packedTemp, packedTemp + setCount);
                }

                return node;
            }

            // Handle internal nodes
            int childScale = scale - 2;  // Decrease scale for children

            // First collect all child nodes and their indices
            struct ChildInfo
            {
                int index;
                RawNode node;
            };

            std::vector<ChildInfo> childrenInfo;

            // Process all 64 possible child cells
            for (int i = 0; i < 64; ++i)
            {
                // Convert 1D index to 3D position
                int x = (i >> 0) & 3;
                int z = (i >> 2) & 3;
                int y = (i >> 4) & 3;

                // Calculate child position
                Vec3 newPos = pos + Vec3(x, y, z) * (1 << childScale);

                // Recursively process child
                RawNode child = GenerateTreeFromGrid(nodes, leafData, childScale, newPos);

                // Only add non-empty children
                if (child.GetPopMask() != 0)
                {
                    childrenInfo.push_back({i, child});
                }
            }

            // Set population mask based on which children exist
            uint64_t popMask = 0;

            for (const auto& info : childrenInfo)
            {
                popMask |= 1ULL << info.index;
            }

            node.SetPopMask(popMask);

            if (!childrenInfo.empty())
            {
                // Reserve space for all children
                size_t nodeStartIndex = nodes.size();
                node.SetChildPtr(static_cast<uint32_t>(nodeStartIndex));

                // Sort children by index for deterministic ordering
                std::sort(childrenInfo.begin(), childrenInfo.end(),
                          [](const ChildInfo& a, const ChildInfo& b)
                {
                    return a.index < b.index;
                });

                // Allocate space for all children at once
                nodes.resize(nodeStartIndex + childrenInfo.size());

                // Add each child at the correct position determined by popcount
                for (const auto& info : childrenInfo)
                {
                    // Calculate the correct index based on population count
                    uint32_t childOffset = popcnt_var64(popMask, info.index);
                    nodes[nodeStartIndex + childOffset] = info.node;
                }
            }

            return node;
        }
};

// Example usage of the Tree64 API:
/*
int main() {
    // Create a sphere shape
    Sphere sphere(Vec3(16, 16, 16), 10.0f);

    // Create a 64-tree with max scale 8 (256³ grid)
    Tree64 tree(8);

    // Initialize from shape
    tree.InitializeFromShape(sphere);

    // Get the generated data for GPU upload
    const OctreeData& data = tree.GetOctreeData();

    // Modify a voxel and regenerate
    tree.ModifyVoxel(Vec3(20, 20, 20), true);

    // Batch modify multiple voxels
    std::vector<Vec3> positions = {Vec3(21, 21, 21), Vec3(22, 22, 22)};
    std::vector<bool> values = {true, false};
    tree.BatchModifyVoxels(positions, values);

    // Access the grid directly for custom operations
    DenseGrid& grid = tree.GetGrid();

    // Create a new grid and replace the existing one
    DenseGrid newGrid(tree.GetGrid().GetSize());
    Cube cube(Vec3(16, 16, 16), 20);
    newGrid.InitializeFromShape(cube);
    tree.ReplaceGrid(newGrid);

    return 0;
}
*/