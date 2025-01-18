#include "shared.inl"

struct CompactPush {
    daxa_BufferPtr(VisibleBricksBuffer) visible_bricks_ptr;
    daxa_BufferPtr(CompactVisibleBricks) compact_bricks_ptr;
};

groupshared uint g_prefix_sum[256];

[numthreads(256, 1, 1)]
void main(uint3 dtid : SV_DispatchThreadID, uint3 gtid : SV_GroupThreadID, uint3 gid : SV_GroupID) 
{
    uint thread_idx = gtid.x;
    uint global_idx = dtid.x;
    
    // First pass: count bits set in this thread's section
    uint total_bricks = GRID_SIZE_CUBE * CHUNK_SIZE_CUBE;
    uint bits_per_thread = (total_bricks + 255) / 256;
    uint start_bit = global_idx * bits_per_thread;
    
    uint local_count = 0;
    for(uint i = 0; i < bits_per_thread && (start_bit + i) < total_bricks; i++) {
        uint bit_idx = start_bit + i;
        uint array_idx = bit_idx / 32;
        uint bit_offset = bit_idx % 32;
        
        if(p.visible_bricks_ptr.bits[array_idx] & (1u << bit_offset)) {
            local_count++;
        }
    }
    
    // Store local count
    g_prefix_sum[thread_idx] = local_count;
    GroupMemoryBarrierWithGroupSync();
    
    // Parallel prefix sum within the group
    for(uint stride = 1; stride < 256; stride *= 2) {
        uint sum = 0;
        if(thread_idx >= stride) {
            sum = g_prefix_sum[thread_idx - stride];
        }
        GroupMemoryBarrierWithGroupSync();
        if(thread_idx >= stride) {
            g_prefix_sum[thread_idx] += sum;
        }
        GroupMemoryBarrierWithGroupSync();
    }
    
    // Write total count from last thread
    if(thread_idx == 255 && gid.x == 0) {
        p.compact_bricks_ptr.count = g_prefix_sum[255];
    }
    
    // Second pass: write compacted brick indices
    uint write_offset = thread_idx > 0 ? g_prefix_sum[thread_idx - 1] : 0;
    
    for(uint i = 0; i < bits_per_thread && (start_bit + i) < total_bricks; i++) {
        uint bit_idx = start_bit + i;
        uint array_idx = bit_idx / 32;
        uint bit_offset = bit_idx % 32;
        
        if(p.visible_bricks_ptr.bits[array_idx] & (1u << bit_offset)) {
            VisibleBrick brick;
            brick.chunk_index = bit_idx / CHUNK_SIZE_CUBE;
            brick.brick_index = bit_idx % CHUNK_SIZE_CUBE;
            p.compact_bricks_ptr.bricks[write_offset++] = brick;
        }
    }
}
