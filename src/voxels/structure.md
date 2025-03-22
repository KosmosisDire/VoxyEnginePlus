# Voxel System Technical Specification


# Voxel System Technical Specification

## Core Structure
- World composed of 6x6x6 voxel blocks
- Blocks organized in 4x4x4 chunks for hierarchical structure
- Each block contains geometry (bitmask) and material data
- System optimized for GPU processing in Slang shading language

## Memory Layout

### Bitmask Structure (32 bytes total)
- 216 bits needed for 6x6x6 voxels
- Stored in 8 uint32s (256 bits) for 16-byte alignment
- Actual data uses 216 bits spanning first 7 uint32s

### Material Data Indexing Options

1. Recommended Layout (Simple):
  * `uint32[7]`: Full 32-bit pointer to material data
  * Last 4 bits of `uint32[6]`: Buffer type index (0-15)
  * Advantages:
    - Direct memory access without bit manipulation
    - GPU-friendly 32-bit aligned reads
    - Better extensibility for future features
    - Simpler implementation and maintenance

2. Alternative Space-Optimized Layout:
  * Last 28 bits of `uint32[7]`: 25-bit pointer (max 19,884,107 blocks) + 3-bit buffer type (0-7)
  * 8 free bits in `uint32[6]` after voxel data
  * 4 free bits at start of `uint32[7]`
  * Total 12 bits available for additional metadata
  * Trade-offs:
    - Requires more complex bit manipulation
    - Small memory savings (~7 bits per block)
    - Potentially worse GPU performance
    - Less flexible for future modifications

### Material Data System
Current (Uncompressed):
- 1 byte per voxel
- 216 bytes per block (6x6x6)
- ~6.75x more memory usage than bitmasks

Palette Compression System:
1. Palette Buffer:
```cpp
struct BlockPalette {
    uint8_t num_materials;  // Number of unique materials
    uint8_t `palette[16]`;    // Material lookup table
};
```

2. Multiple Index Buffers:
- Separate buffers for different bit-widths
- Example sizes:
  * 4-bit buffer: 108 bytes per block (216 indices)
  * 5-bit buffer: 135 bytes per block
  * etc.
- Each block references appropriate buffer using type index

## Memory Requirements Examples
For 160x160x160 block world:
- Total blocks: 4,096,000
- Bitmask memory: 125 MB
- Uncompressed material memory: 844 MB
- Total uncompressed: 969 MB

Reference points:
- 1GB bitmask memory = 323x323x323 block world
- Maximum blocks possible = 19,884,107 (limited by uint32 max value / voxels per block)
  * This allows for roughly 270x270x270 blocks (≈1,620x1,620x1,620 voxels)

## GPU Access Patterns
1. Geometry Access:
- Reads 7 uint32s for bitmask data
- Last uint32 provides material data location
- Buffer type stored in last 4 bits of `uint32[6]`

2. Material Access:
- Thread reads single voxel material:
  1. Get pointer and buffer type from bitmask
  2. Look up in appropriate index buffer
  3. Extract packed index bits
  4. Use index to look up in palette

## Memory Management
- Fixed-size block structure for bitmasks
- Variable-size compressed material data
- Indirection through pointers in bitmask
- Multiple buffers for different compression ratios

## Performance Considerations
1. Memory Access:
- Coalesced reads possible for consecutive blocks
- Single-thread sequential reads not automatically coalesced
- Consider using vector loads where available

2. Compression Trade-offs:
- Palette size vs compression ratio
- Access speed vs memory usage
- Fixed vs variable size structures

3. Key Metrics:
- Uncompressed block = 248 bytes (32 bitmask + 216 materials)
- Compressed size varies with material complexity
- Padding overhead ~15% for bitmask alignment

## Current Development State
- Bitmask structure finalized
- Material compression system designed but not implemented
- Memory management system needed for compressed data
- GPU access patterns optimized for Slang shader environment

## Next Steps
1. Implement palette compression system
2. Develop memory management for compressed data
3. Optimize GPU access patterns
4. Test with various world sizes and material distributions