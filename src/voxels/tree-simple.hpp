#pragma once

#include <vector>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <algorithm>
#include "tree-shapes.hpp"

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

    // Accessors and mutators
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

// Helper function to get cell index from position
inline int GetNodeCellIndex(const Vec3& pos, int scale_exp)
{
    Vec3 cellPos = (pos >> scale_exp) & 3;
    return cellPos.x + (cellPos.z * 4) + (cellPos.y * 16);
}

// Count set bits in mask up to position
inline uint32_t popcnt_var64(uint64_t mask, uint32_t pos)
{
    if (pos == 0)
    {
        return 0;
    }

    uint64_t bitMask = (1ULL << pos) - 1ULL;

    // Use compiler built-in if available, otherwise implement manually
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_popcountll(mask & bitMask);
#else
    // Simple fallback implementation
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

// Platform-agnostic popcount implementation
inline uint32_t popcount(uint64_t value)
{
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_popcountll(value);
#else
    // Simple fallback implementation
    uint32_t count = 0;

    while (value)
    {
        count += value & 1;
        value >>= 1;
    }

    return count;
#endif
}

// Compress an array by removing elements where mask bit is 0
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

// Main tree generation function
RawNode GenerateTree(
    std::vector<RawNode>& nodes,
    std::vector<uint8_t>& leafData,
    const Shape& shape,
    int32_t scale,
    const Vec3& pos = Vec3(0, 0, 0)
)
{
    // Initialize empty node
    RawNode node;

    // Skip if completely outside shape
    if (!shape.Intersects(pos, scale))
    {
        return node;
    }

    // Create leaf node at minimum scale
    if (scale == 2)
    {
        uint8_t temp[64] = {0};

        if (shape.ContainsCompletely(pos, scale))
        {
            // Fill all voxels
            std::memset(temp, 255, 64);
        }
        else
        {
            // Test each individual voxel - using exact index calculation from shader
            for (int y = 0; y < 4; ++y)
            {
                for (int z = 0; z < 4; ++z)
                {
                    for (int x = 0; x < 4; ++x)
                    {
                        // Use exactly the same index formula as in shader's GetNodeCellIndex
                        int index = x + (z * 4) + (y * 16);

                        // Test voxel center (add 0.5 to get center)
                        Vec3 voxelPos(pos.x + x, pos.y + y, pos.z + z);
                        Vec3 voxelCenter(voxelPos.x + 0.5f, voxelPos.y + 0.5f, voxelPos.z + 0.5f);

                        if (shape.Contains(voxelCenter))
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

            // Pack the data exactly as the shader expects
            uint32_t leafPtr = static_cast<uint32_t>(leafData.size());
            node.SetChildPtr(leafPtr);

            // Critical: use LeftPack to arrange leaf data in the expected order
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
        RawNode child = GenerateTree(nodes, leafData, shape, childScale, newPos);

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

        // Sort children by index to ensure correct traversal
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

// Main entry point function
OctreeData GenerateVoxelData(const Shape& shape, uint32_t maxScale = 10)
{
    OctreeData result;

    // Reserve space for efficiency
    result.nodes.reserve(1000);
    result.leafData.reserve(1000);

    // Ensure space for root node
    result.nodes.resize(1);

    // Generate tree starting at root
    result.nodes[0] = GenerateTree(result.nodes, result.leafData, shape, maxScale, Vec3(0, 0, 0));

    return result;
}