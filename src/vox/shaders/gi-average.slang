#include "common.slang"

static const float3[] normals = {
    float3(1, 0, 0),
    float3(-1, 0, 0),
    float3(0, 1, 0),
    float3(0, -1, 0),
    float3(0, 0, 1),
    float3(0, 0, -1)
};

[numthreads(256, 1, 1)]
void main(uint3 dispatch_id: SV_DispatchThreadID)
{
    uint visible_index = dispatch_id.x;
    if (visible_index >= p.compact_visible_ptr.count)
        return;

    uint brick_index = p.compact_visible_ptr.indices[visible_index];

    // for all 6 adjacent bricks
    for (uint i = 0; i < 6; i++)
    {
        float3 normal = normals[i];
        uint adjacent_brick_index = getAdjacentBrickIndex(brick_index, normal);

        if (adjacent_brick_index < 0 || adjacent_brick_index >= GRID_SIZE_CUBE * CHUNK_SIZE_CUBE)
            continue;

        int sample_count;
        InterlockedExchange(p.brick_data_ptr.rayCount[adjacent_brick_index], 0, sample_count);
        if (sample_count > 0)
        {
            // Average the accumulated light values
            p.brick_data_ptr.light[adjacent_brick_index].xyz /= sample_count;
        }
    }
}
