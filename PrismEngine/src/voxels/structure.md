# NOTE:
This was written by an AI based on a handwritten spec, and it represents the existing design of the contree.
The contree code needs to be reviewed to ensure code quality.
However the contree building code and the contree tracing code was not written by the AI, so it should be correct.



# Contree Data Structure Overview

## High-Level Architecture

```
Contree (4 levels, 4x4x4 branching)
├─ Node Buffer        (12 bytes each)
├─ Brick Buffer       (8 bytes each - occupancy only)
├─ Palette Buffer     (variable uint16s)
└─ Material Indices   (variable uint16s - per leaf)
```

## 1. Node Structure (12 bytes)

### Internal Node Layout
```
Byte 0-3:  IsLeaf(0) | IsAbsolutePtr(1) | ChildPtr(30)
Byte 4-7:  PopMask[31:0]   (low 32 bits)
Byte 8-11: PopMask[63:32]  (high 32 bits)
```

**PopMask encoding**: Each bit represents one of 64 possible children (4x4x4)
- Bit position = x + y*4 + z*16
- Bit set = child exists
- Bit clear = child empty/missing

### Leaf Node Layout (Same 12 bytes, different interpretation)
```
Byte 0-3:  IsLeaf(1) | flags(1) | OccupancyPtr(30)
Byte 4-7:  PalettePtr(32)
Byte 8-11: MaterialIndicesPtr(32)
```

**Key insight**: Leaves reuse the PopMask space for two additional pointers since leaves have no children.

## 2. Brick Structure (8 bytes)

```cpp
struct Brick {
    uint64_t occupancy;  // 4x4x4 = 64 bits
};
```

**Occupancy encoding**: Each bit represents one voxel in 4x4x4 brick
- Bit position = x + y*4 + z*16
- Bit set = voxel occupied
- Bit clear = voxel empty

**Important**: Multiple leaves can share the same brick (deduplication by topology)

## 3. Palette Buffer (Variable size)

Current simple format (uncompressed):
```
Offset 0:    material_count (uint16_t)
Offset 2:    material[0]    (uint16_t)
Offset 4:    material[1]    (uint16_t)
...
Offset 2+N*2: material[N-1]  (uint16_t)
```

**Example - Palette 0:**
```
[0x0002] [0x0000] [0x0001]
   ↑        ↑        ↑
 count=2  mat0=0   mat1=1
```

**Important**: Multiple leaves can share the same palette

## 4. Material Indices Buffer (Variable size)

Current format (uncompressed):
```
64 × uint16_t = 128 bytes per leaf
```

Each uint16_t is an index into the palette for that voxel.

**Voxel ordering**: index = x + y*4 + z*16

**Important**: Each leaf has unique material indices (NOT shared)

---

## Complete Data Access Pattern

### Step 1: Tree Traversal to Leaf

```cpp
// Start at root (index 0)
uint32_t node_idx = 0;
ContreeNode node = nodes[node_idx];

while (!node.IsLeaf()) {
    // Compute which child we need (based on ray position)
    int child_x = ...; // 0-3
    int child_y = ...; // 0-3  
    int child_z = ...; // 0-3
    
    uint32_t child_bit = child_x + child_y*4 + child_z*16;
    
    // Check if child exists
    uint64_t popMask = node.GetPopMask();
    if (!(popMask & (1ull << child_bit))) {
        return MISS; // Child doesn't exist
    }
    
    // Find child's index using prefix popcount
    uint32_t child_offset = Helpers::prefix_popcnt64(popMask, child_bit);
    uint32_t child_ptr = node.GetChildPtr();
    
    node_idx = child_ptr + child_offset;
    node = nodes[node_idx];
}

// Now we have a leaf node
assert(node.IsLeaf());
```

### Step 2: Test Brick Occupancy

```cpp
// Get brick occupancy (SHARED - multiple leaves may point to same brick)
uint32_t brick_ptr = node.GetOccupancyPtr();
Brick brick = bricks[brick_ptr];

// Compute which voxel in the 4x4x4 brick
int local_x = ...; // 0-3
int local_y = ...; // 0-3
int local_z = ...; // 0-3

uint32_t voxel_idx = local_x + local_y*4 + local_z*16;

// Test occupancy
if (!brick.TestOccupancy(voxel_idx)) {
    return MISS; // Voxel is empty
}

// HIT - voxel is occupied, now get material
```

### Step 3: Fetch Material Data (On Hit Only)

```cpp
// Get palette pointer (SHARED - multiple leaves may use same palette)
uint32_t palette_ptr = node.GetPalettePtr();

// Read material count from palette
uint16_t material_count = paletteBuffer[palette_ptr];

// Get palette array (skip count)
const uint16_t* palette = &paletteBuffer[palette_ptr + 1];

// Get material indices pointer (UNIQUE per leaf)
uint32_t indices_ptr = node.GetMaterialIndicesPtr();

// Read material index for this voxel
uint16_t material_idx = materialIndices[indices_ptr + voxel_idx];

// Look up actual material ID from palette
uint16_t material_id = palette[material_idx];

// Now fetch material properties from global material table
Material mat = material_table[material_id];
```

---

## Memory Access Summary

### Empty Space Traversal (Common Case)
1. ✓ Fetch node (12 bytes)
2. ✓ Test PopMask (already in node)
3. ✓ Compute child offset (CPU work)
4. → Repeat until leaf or miss

**Material data NOT touched during traversal**

### Leaf Hit with Empty Voxel
1. ✓ Fetch leaf node (12 bytes)
2. ✓ Fetch brick (8 bytes) - via OccupancyPtr
3. ✓ Test occupancy bit
4. → Miss, no material fetch

### Leaf Hit with Solid Voxel
1. ✓ Fetch leaf node (12 bytes)
2. ✓ Fetch brick (8 bytes) - via OccupancyPtr
3. ✓ Test occupancy bit → HIT
4. ✓ Fetch palette header (2 bytes) - via PalettePtr
5. ✓ Fetch palette entry (2 bytes)
6. ✓ Fetch material index (2 bytes) - via MaterialIndicesPtr
7. ✓ Fetch material properties (from global table)

---

## Deduplication Strategy

### Occupancy Sharing
```
Leaf A → OccupancyPtr = 0 → Brick[0] (all solid)
Leaf B → OccupancyPtr = 0 → Brick[0] (SAME)
Leaf C → OccupancyPtr = 5 → Brick[5] (different pattern)
```

### Palette Sharing
```
Leaf A → PalettePtr = 0 → [count=2, mat0=0, mat1=1]
Leaf B → PalettePtr = 0 → SAME palette
Leaf C → PalettePtr = 10 → [count=4, mat0=3, mat1=5, mat2=7, mat3=9]
```

### Material Indices (NO Sharing)
```
Leaf A → MaterialIndicesPtr = 0    → [0,1,0,1,1,0,1,0,...]  (64 indices)
Leaf B → MaterialIndicesPtr = 64   → [1,0,1,0,0,1,0,1,...]  (64 indices)
Leaf C → MaterialIndicesPtr = 128  → [0,0,1,1,2,2,3,3,...]  (64 indices)
```

---

## Example: Concrete Memory Layout

### Nodes Buffer
```
Offset 0:   [Root Node] IsLeaf=0, PopMask=0x..., ChildPtr=1
Offset 12:  [Node 1]    IsLeaf=0, PopMask=0x..., ChildPtr=65
Offset 24:  [Node 2]    IsLeaf=1, OccPtr=0, PalPtr=0, IdxPtr=0
...
```

### Bricks Buffer
```
Offset 0:  [Brick 0] occupancy = 0xFFFFFFFFFFFFFFFF (all solid)
Offset 8:  [Brick 1] occupancy = 0x...             (custom pattern)
...
```

### Palette Buffer
```
Offset 0:  [Palette 0] 0x0002, 0x0000, 0x0001  (count=2, [0,1])
Offset 6:  [Palette 1] 0x0004, 0x0003, 0x0005, 0x0007, 0x0009  (count=4, [3,5,7,9])
...
```

### Material Indices Buffer
```
Offset 0:   [Leaf 0 indices] 64 × uint16_t (128 bytes)
Offset 128: [Leaf 1 indices] 64 × uint16_t (128 bytes)
Offset 256: [Leaf 2 indices] 64 × uint16_t (128 bytes)
...
```

---

## Key Design Benefits

1. **Occupancy separation**: Can traverse and test occupancy without touching material data
2. **Three-level deduplication**: Occupancy, palettes, and indices can be shared independently
3. **Compact leaves**: Only 12 bytes per leaf, stores 3 pointers by reusing PopMask space
4. **Cache-friendly**: Occupancy tests (hot path) are small and separate from material data
5. **Flexible compression**: TODO - Palette and indices can be compressed without changing node structure

---

## TODO: Future Compression

When implementing palette compression:

### Compressed Material Indices Buffer
```
Offset 0:  bit_width (computed from material_count)
           packed_bits (variable size)
```

**Size calculation:**
- 1 material: 0 bits per voxel (all same)
- 2 materials: 1 bit per voxel = 8 bytes
- 4 materials: 2 bits per voxel = 16 bytes
- 8 materials: 3 bits per voxel = 24 bytes
- 16 materials: 4 bits per voxel = 32 bytes

Access becomes bit extraction instead of direct indexing.