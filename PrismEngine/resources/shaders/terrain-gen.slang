#include "common.slang"

float3 grid_to_world_pos(int3 grid_pos) {
    return float3(grid_pos) * WORLD_VOXEL_SIZE;
}

// Simple heightmap - just a sine wave pattern
float calculate_terrain_height(float2 position) {
    float base_height = 20.0f;
    float wave = sin(position.x * 0.1f) * cos(position.y * 0.1f) * 5.0f;
    return base_height + wave;
}

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

    // Initialize brick palette
    p.bricksBuffer.data[0].data = {0, 0, 0, 0, 0, 0, 0, 0};
    p.bricksBuffer.data[1].data = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
                                   0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};

    if (p.passNum == 1) {
        // Simple heightmap terrain generation
        int3 abs_voxel = brick_coord * BRICK_SIZE;
        float3 world_pos = grid_to_world_pos(abs_voxel);
        float terrain_height = calculate_terrain_height(float2(world_pos.x, world_pos.z));

        bool is_solid = world_pos.y < terrain_height;

        if (is_solid) {
            p.materialPtrBuffer.data[brick_index] = 1; // Grass material
            p.brickPtrBuffer.data[brick_index] = 1;
            setBit(p.chunksBuffer, chunk_index, local_brick_index);
        } else {
            p.brickPtrBuffer.data[brick_index] = 0;
            clearBit(p.chunksBuffer, chunk_index, local_brick_index);
        }
    }
    else if (p.passNum == 2) {
        // Skip second pass for now
    }
}
