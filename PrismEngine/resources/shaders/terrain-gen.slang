#include "common.slang"
#include "lygia/generative/random.hlsl"
#include "lygia/generative/snoise.hlsl"

// #region PRNG

uint hash(uint x) {
    x ^= x >> 16;
    x *= 0x85ebca6b;
    x ^= x >> 13;
    x *= 0xc2b2ae35;
    x ^= x >> 16;
    return x;
}

uint hash2(uint x, uint y) {
    return hash(x ^ hash(y));
}

uint hash3(uint x, uint y, uint z) {
    return hash(x ^ hash(y ^ hash(z)));
}

float rand_float(inout uint seed) {
    seed = hash(seed);
    return float(seed) / float(0xFFFFFFFF);
}

float rand_range(inout uint seed, float min_val, float max_val) {
    return min_val + rand_float(seed) * (max_val - min_val);
}

int rand_int(inout uint seed, int min_val, int max_val) {
    float r = rand_float(seed);
    return min_val + int(floor(r * float(max_val - min_val + 1)));
}

float3 rand_unit_vector(inout uint seed) {
    float theta = rand_range(seed, 0.0f, 2.0f * 3.14159f);
    float phi = rand_range(seed, -1.0f, 1.0f);
    float r = sqrt(1.0f - phi * phi);
    return float3(r * cos(theta), phi, r * sin(theta));
}

// #endregion

// #region Basic Helpers

float3 grid_to_world_pos(int3 grid_pos) {
    return float3(grid_pos) * WORLD_VOXEL_SIZE;
}

int3 world_to_grid_pos(float3 world_pos) {
    return int3(floor(world_pos / WORLD_VOXEL_SIZE));
}

float simple_noise(float2 pos, float scale, float amplitude) {
    return snoise(float3(pos.x * scale, pos.y * scale, 0.0f)) * amplitude;
}

float fbm_simple(float2 pos, float scale, float amplitude, int octaves) {
    float value = 0.0f;
    float freq = scale;
    float amp = amplitude;
    
    for (int i = 0; i < octaves; i++) {
        value += simple_noise(pos, freq, amp);
        freq *= 2.0f;
        amp *= 0.5f;
    }
    
    return value;
}

// #endregion

// #region Terrain Generation

float plateau_noise(float2 pos) {
    float base = simple_noise(pos, 0.008f, 1.0f);
    if (abs(base) < 0.3f) {
        base = sign(base) * 0.3f;
    }
    return base;
}

float erosion_factor(float2 pos) {
    float erosion = simple_noise(pos, 0.005f, 1.0f);
    erosion = pow(abs(erosion), 0.5f) * sign(erosion);
    return erosion;
}

float calculate_terrain_height(float2 position) {
    float base_height = 64.0f;
    
    float continental = plateau_noise(position) * 8.0f;
    float erosion = erosion_factor(position);
    float gentle_hills = simple_noise(position, 0.02f, 16.0f);
    
    float flat_mask = simple_noise(position, 0.003f, 1.0f);
    if (abs(flat_mask) < 0.4f) {
        gentle_hills *= 0.2f;
    }
    
    float mesa_mask = simple_noise(position + float2(100.0f, 100.0f), 0.004f, 1.0f);
    float height = base_height + continental + gentle_hills;
    
    if (mesa_mask > 0.3f) {
        float step_size = 4.0f;
        height = floor(height / step_size) * step_size;
        height += simple_noise(position, 0.1f, 0.5f);
    }
    
    if (erosion < -0.5f) {
        height -= (abs(erosion) - 0.5f) * 10.0f;
    }
    
    float detail = simple_noise(position, 0.15f, 0.5f);
    
    return height + detail;
}

uint get_terrain_material(int3 voxel_pos) {
    float3 world_pos = grid_to_world_pos(voxel_pos);
    float2 terrain_pos = float2(world_pos.x, world_pos.z);
    float terrain_height = calculate_terrain_height(terrain_pos);
    
    float depth = terrain_height - world_pos.y;
    
    float mesa_check = simple_noise(terrain_pos, 0.004f, 1.0f);
    
    if (mesa_check > 0.3f) {
        int layer = (int)(world_pos.y) % 8;
        if (layer < 2) {
            return 5;
        } else if (layer < 4) {
            return 2;
        } else if (layer < 6) {
            return 11;
        } else {
            return 5;
        }
    }
    
    if (depth > 8.0f) {
        return 2; // Stone
    }
    
    if (depth > 2.0f) {
        return 5; // Dirt
    }
    
    float erosion = erosion_factor(terrain_pos);
    if (erosion < -0.6f) {
        return 5; // Exposed dirt
    }
    
    return 1; // Grass
}

// #endregion

// #region Biome Detection

enum BiomeType {
    BIOME_PLAINS = 0,
    BIOME_FOREST = 1,
    BIOME_DENSE_FOREST = 2,
    BIOME_SPARSE = 3,
    BIOME_MESA = 4
};

BiomeType get_biome(float2 world_pos) {
    float biome_noise = simple_noise(world_pos, 0.003f, 1.0f);
    float moisture = simple_noise(world_pos + float2(1000.0f, 0.0f), 0.004f, 1.0f);
    
    float mesa_check = simple_noise(world_pos + float2(100.0f, 100.0f), 0.004f, 1.0f);
    if (mesa_check > 0.3f) {
        return BiomeType.BIOME_MESA;
    }
    
    if (moisture > 0.3f) {
        if (biome_noise > 0.2f) {
            return BiomeType.BIOME_DENSE_FOREST;
        } else {
            return BiomeType.BIOME_FOREST;
        }
    } else if (moisture < -0.3f) {
        return BiomeType.BIOME_SPARSE;
    }
    
    return BiomeType.BIOME_PLAINS;
}

// #endregion

// #region Tree Structure

struct tree_params {
    float3 base_pos;
    uint seed;
    int type;
    float trunk_height;
    float trunk_radius;
    int branch_count;
    float branch_spread;
    float leaf_density;
    float leaf_radius;
    bool has_roots;
};

bool should_place_tree_at_brick(int3 brick_coord) {
    float3 world_pos = grid_to_world_pos(brick_coord * BRICK_SIZE);
    float2 terrain_pos = float2(world_pos.x, world_pos.z);
    
    // Get terrain height and check basic constraints
    float terrain_height = calculate_terrain_height(terrain_pos);
    if (terrain_height < 62.0f || world_pos.y > terrain_height + 1.0f) {
        return false;
    }
    
    // Check slope
    float slope_check = 4.0f;
    float h1 = calculate_terrain_height(terrain_pos + float2(slope_check, 0));
    float h2 = calculate_terrain_height(terrain_pos + float2(-slope_check, 0));
    float h3 = calculate_terrain_height(terrain_pos + float2(0, slope_check));
    float h4 = calculate_terrain_height(terrain_pos + float2(0, -slope_check));
    float max_slope = max(max(abs(h1 - terrain_height), abs(h2 - terrain_height)), 
                          max(abs(h3 - terrain_height), abs(h4 - terrain_height)));
    
    if (max_slope > 3.0f) {
        return false;
    }
    
    // Get biome and determine tree density
    BiomeType biome = get_biome(terrain_pos);
    float tree_chance = 0.0f;
    
    switch (biome) {
        case BiomeType.BIOME_PLAINS:
            tree_chance = 0.02f;
            break;
        case BiomeType.BIOME_FOREST:
            tree_chance = 0.15f;
            break;
        case BiomeType.BIOME_DENSE_FOREST:
            tree_chance = 0.3f;
            break;
        case BiomeType.BIOME_SPARSE:
            tree_chance = 0.005f;
            break;
        case BiomeType.BIOME_MESA:
            tree_chance = 0.001f;
            break;
    }
    
    // Use deterministic placement
    uint pos_hash = hash3(uint(brick_coord.x), uint(brick_coord.y), uint(brick_coord.z));
    float placement_roll = float(pos_hash) / float(0xFFFFFFFF);
    
    return placement_roll < tree_chance;
}

tree_params get_tree_params(int3 brick_coord) {
    tree_params params;
    
    float3 world_pos = grid_to_world_pos(brick_coord * BRICK_SIZE);
    float2 terrain_pos = float2(world_pos.x, world_pos.z);
    float terrain_height = calculate_terrain_height(terrain_pos);
    
    params.base_pos = float3(world_pos.x, terrain_height, world_pos.z);
    params.seed = hash3(uint(brick_coord.x), uint(brick_coord.y), uint(brick_coord.z));
    
    BiomeType biome = get_biome(terrain_pos);
    uint seed = params.seed;
    float type_roll = rand_float(seed);
    
    // Select tree type based on biome
    if (biome == BiomeType.BIOME_DENSE_FOREST || biome == BiomeType.BIOME_FOREST) {
        if (type_roll < 0.4f) {
            // Oak - branchy tree
            params.type = 0;
            params.trunk_height = rand_range(seed, 12.0f, 20.0f);
            params.trunk_radius = rand_range(seed, 1.0f, 1.5f);
            params.branch_count = rand_int(seed, 3, 6);
            params.branch_spread = 45.0f;
            params.leaf_density = 0.7f;
            params.leaf_radius = 4.0f;
            params.has_roots = true;
        } else if (type_roll < 0.7f) {
            // Birch - tall
            params.type = 1;
            params.trunk_height = rand_range(seed, 18.0f, 28.0f);
            params.trunk_radius = rand_range(seed, 0.8f, 1.0f);
            params.branch_count = rand_int(seed, 2, 4);
            params.branch_spread = 25.0f;
            params.leaf_density = 0.6f;
            params.leaf_radius = 3.0f;
            params.has_roots = false;
        } else {
            // Spruce
            params.type = 2;
            params.trunk_height = rand_range(seed, 20.0f, 30.0f);
            params.trunk_radius = rand_range(seed, 0.8f, 1.2f);
            params.branch_count = rand_int(seed, 8, 12);
            params.branch_spread = 20.0f;
            params.leaf_density = 0.8f;
            params.leaf_radius = 2.5f;
            params.has_roots = false;
        }
    } else {
        // Sparse trees for other biomes
        params.type = 3;
        params.trunk_height = rand_range(seed, 8.0f, 15.0f);
        params.trunk_radius = rand_range(seed, 1.0f, 1.8f);
        params.branch_count = rand_int(seed, 2, 4);
        params.branch_spread = 50.0f;
        params.leaf_density = 0.5f;
        params.leaf_radius = 3.5f;
        params.has_roots = true;
    }
    
    return params;
}

// #endregion

// #region Tree Generation

void set_voxel_if_in_bounds(int3 grid_pos, uint material) {
    int world_block_span = GRID_SIZE * CHUNK_SIZE;
    
    if (grid_pos.x >= 0 && grid_pos.x < world_block_span &&
        grid_pos.y >= 0 && grid_pos.y < world_block_span &&
        grid_pos.z >= 0 && grid_pos.z < world_block_span) {
        
        int3 chunk_coord = grid_pos / CHUNK_SIZE;
        int3 local_brick_coord = (grid_pos % CHUNK_SIZE) / BRICK_SIZE;
        
        int chunk_index = chunkIndex(chunk_coord);
        int local_brick_index = brickLocalIndex(local_brick_coord);
        int brick_index = brickGlobalIndex(chunk_index, local_brick_index);
        
        p.brickPtrBuffer.data[brick_index] = 1;
        p.materialPtrBuffer.data[brick_index] = (uint8_t)material;
        setBit(p.chunksBuffer, chunk_index, local_brick_index);
    }
}

void generate_trunk(tree_params params) {
    uint seed = params.seed;
    float3 current_pos = params.base_pos;
    float current_radius = params.trunk_radius;
    
    // Add roots
    if (params.has_roots) {
        int root_count = rand_int(seed, 3, 5);
        for (int r = 0; r < root_count; r++) {
            float angle = (float(r) / float(root_count)) * 2.0f * 3.14159f + rand_range(seed, -0.3f, 0.3f);
            float root_radius = current_radius * 0.6f;
            
            for (int i = 0; i < 8; i++) {
                float3 root_pos = params.base_pos + float3(
                    cos(angle) * float(i) * 0.8f,
                    -float(i) * 0.3f,
                    sin(angle) * float(i) * 0.8f
                );
                
                int radius_voxels = int(ceil(root_radius));
                for (int dx = -radius_voxels; dx <= radius_voxels; dx++) {
                    for (int dy = -radius_voxels; dy <= radius_voxels; dy++) {
                        for (int dz = -radius_voxels; dz <= radius_voxels; dz++) {
                            float dist = sqrt(float(dx*dx + dy*dy + dz*dz));
                            if (dist <= root_radius) {
                                int3 voxel_pos = world_to_grid_pos(root_pos) + int3(dx, dy, dz);
                                set_voxel_if_in_bounds(voxel_pos, 3); // Wood
                            }
                        }
                    }
                }
                
                root_radius *= 0.85f;
                if (root_radius < 0.5f) break;
            }
        }
    }
    
    // Generate main trunk with slight curves
    int trunk_segments = int(params.trunk_height);
    float3 trunk_dir = float3(0, 1, 0);
    
    for (int i = 0; i < trunk_segments; i++) {
        // Add slight random lean
        if (i % 4 == 0) {
            trunk_dir = normalize(trunk_dir + rand_unit_vector(seed) * 0.1f);
            trunk_dir.y = abs(trunk_dir.y); // Keep going up
        }
        
        current_pos += trunk_dir;
        
        // Taper trunk
        if (i > trunk_segments / 2) {
            current_radius *= 0.98f;
        }
        
        // Fill trunk cylinder
        int radius_voxels = int(ceil(current_radius));
        for (int dx = -radius_voxels; dx <= radius_voxels; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                for (int dz = -radius_voxels; dz <= radius_voxels; dz++) {
                    float dist = sqrt(float(dx*dx + dz*dz));
                    if (dist <= current_radius) {
                        int3 voxel_pos = world_to_grid_pos(current_pos) + int3(dx, dy, dz);
                        set_voxel_if_in_bounds(voxel_pos, 3); // Wood
                    }
                }
            }
        }
    }
}

void generate_branch(float3 start_pos, float3 direction, float length, float radius, int depth, uint seed) {
    if (depth > 2 || radius < 0.3f) {
        // End with leaves
        int leaf_radius = int(ceil(radius * 4.0f));
        for (int dx = -leaf_radius; dx <= leaf_radius; dx++) {
            for (int dy = -leaf_radius; dy <= leaf_radius; dy++) {
                for (int dz = -leaf_radius; dz <= leaf_radius; dz++) {
                    float dist = sqrt(float(dx*dx + dy*dy + dz*dz));
                    if (dist <= float(leaf_radius)) {
                        uint leaf_hash = hash3(uint(dx + 100), uint(dy + 100), uint(dz + 100)) ^ seed;
                        if (float(leaf_hash) / float(0xFFFFFFFF) < 0.7f) {
                            int3 voxel_pos = world_to_grid_pos(start_pos) + int3(dx, dy, dz);
                            set_voxel_if_in_bounds(voxel_pos, 4); // Leaves
                        }
                    }
                }
            }
        }
        return;
    }
    
    float3 current_pos = start_pos;
    int segments = int(length);
    
    for (int i = 0; i < segments; i++) {
        current_pos += direction;
        
        // Branch voxels
        int radius_voxels = int(ceil(radius));
        for (int dx = -radius_voxels; dx <= radius_voxels; dx++) {
            for (int dy = -radius_voxels; dy <= radius_voxels; dy++) {
                for (int dz = -radius_voxels; dz <= radius_voxels; dz++) {
                    float dist = sqrt(float(dx*dx + dy*dy + dz*dz));
                    if (dist <= radius) {
                        int3 voxel_pos = world_to_grid_pos(current_pos) + int3(dx, dy, dz);
                        set_voxel_if_in_bounds(voxel_pos, 3); // Wood
                    }
                }
            }
        }
        
        // Spawn sub-branches
        if (i == segments / 2 && rand_float(seed) < 0.5f) {
            float3 sub_dir = normalize(direction + rand_unit_vector(seed) * 0.5f);
            //generate_branch(current_pos, sub_dir, length * 0.6f, radius * 0.6f, depth + 1, seed + uint(i));
        }
    }
    
    // Leaf cluster at end
    //generate_branch(current_pos, direction, 0, radius * 0.8f, 3, seed);
}

void generate_tree(tree_params params) {
    uint seed = params.seed;
    
    // Generate trunk
    generate_trunk(params);
    
    // Generate branches
    float branch_start_height = params.trunk_height * 0.5f;
    float branch_spacing = params.trunk_height / float(params.branch_count + 1);
    
    for (int i = 0; i < params.branch_count; i++) {
        float height = branch_start_height + float(i) * branch_spacing;
        float3 branch_pos = params.base_pos + float3(0, height, 0);
        
        float angle = rand_range(seed, 0, 2.0f * 3.14159f);
        float3 branch_dir = normalize(float3(
            cos(angle) * sin(params.branch_spread * 3.14159f / 180.0f),
            cos(params.branch_spread * 3.14159f / 180.0f),
            sin(angle) * sin(params.branch_spread * 3.14159f / 180.0f)
        ));
        
        float branch_length = rand_range(seed, params.trunk_height * 0.3f, params.trunk_height * 0.5f);
        float branch_radius = params.trunk_radius * rand_range(seed, 0.3f, 0.5f);
        
        generate_branch(branch_pos, branch_dir, branch_length, branch_radius, 0, seed + uint(i * 1000));
    }
    
    // Add canopy
    if (params.type == 0 || params.type == 3) {
        float3 canopy_center = params.base_pos + float3(0, params.trunk_height, 0);
        int canopy_radius = int(ceil(params.leaf_radius * 1.5f));
        
        for (int dx = -canopy_radius; dx <= canopy_radius; dx++) {
            for (int dy = -canopy_radius / 2; dy <= canopy_radius; dy++) {
                for (int dz = -canopy_radius; dz <= canopy_radius; dz++) {
                    float dist = sqrt(float(dx*dx + dy*dy + dz*dz));
                    if (dist <= float(canopy_radius)) {
                        uint leaf_hash = hash3(uint(dx + 200), uint(dy + 200), uint(dz + 200)) ^ seed;
                        if (float(leaf_hash) / float(0xFFFFFFFF) < params.leaf_density) {
                            int3 voxel_pos = world_to_grid_pos(canopy_center) + int3(dx, dy, dz);
                            set_voxel_if_in_bounds(voxel_pos, 4); // Leaves
                        }
                    }
                }
            }
        }
    }
}

// #endregion

// #region Main

[numthreads(4, 4, 4)]
void main(uint3 brick_coord: SV_DispatchThreadID, uint3 local_brick_coord: SV_GroupThreadID) {
    int world_block_span = GRID_SIZE * CHUNK_SIZE;

    if (brick_coord.x >= world_block_span || 
        brick_coord.y >= world_block_span || 
        brick_coord.z >= world_block_span)
        return;

    int3 chunk_coord = int3(brick_coord / CHUNK_SIZE);
    int chunk_index = chunkIndex(chunk_coord);
    int local_brick_index = brickLocalIndex(local_brick_coord);
    int brick_index = brickGlobalIndex(chunk_index, local_brick_index);

    // Initialize bricks
    p.bricksBuffer.data[0].data = {0, 0, 0, 0, 0, 0, 0, 0};
    p.bricksBuffer.data[1].data = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
                                   0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
    
    if (p.passNum == 1) {
        // Pass 1: Generate terrain
        int3 abs_voxel = brick_coord * BRICK_SIZE;
        float3 world_pos = grid_to_world_pos(abs_voxel);
        float terrain_height = calculate_terrain_height(float2(world_pos.x, world_pos.z));
        
        bool is_solid = world_pos.y < terrain_height;
        
        if (is_solid) {
            uint material = get_terrain_material(abs_voxel);
            p.materialPtrBuffer.data[brick_index] = (uint8_t)material;
            p.brickPtrBuffer.data[brick_index] = 1;
            setBit(p.chunksBuffer, chunk_index, local_brick_index);
        } else {
            p.brickPtrBuffer.data[brick_index] = 0;
            clearBit(p.chunksBuffer, chunk_index, local_brick_index);
        }
    }
    else if (p.passNum == 2) {
        // Pass 2: Generate trees
        if (true) {
            tree_params params = get_tree_params(brick_coord);
            generate_tree(params);
        }
    }
}

// #endregion