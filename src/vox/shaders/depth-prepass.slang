#include "atmosphere.slang"
#include "raycast.slang"

[numthreads(8, 8, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    RWTexture2D<float> render_image = RWTexture2D<float>::get(p.depth_prepass);
    uint2 pixel = pixel_i.xy;
    if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
        return;

    // Ray setup
    float3 rayOrigin, rayDir;
    getRayFromPixel(pixel, p.frame_dim / 2, p.state_ptr.camera, rayOrigin, rayDir);

    TraceResult trace = traceVoxelRay(rayOrigin, rayDir);

    if (trace.hit)
    {
        render_image[pixel] = trace.distance;

        // track visible bricks this frame

        // Check if brick is already visible this frame
        uint global_idx = brickGlobalIndex(trace.chunkIndex, trace.brickIndex);
        uint bit_idx = global_idx % 32;
        uint array_idx = global_idx / 32;

        uint original;
        InterlockedOr(p.visible_bricks_ptr.bits[array_idx], 1u << bit_idx, original);

        // If bit wasn't set before (brick newly visible this frame)
        if ((original & (1u << bit_idx)) == 0)
        {
            // Add global index to visible list
            uint write_idx;
            InterlockedAdd(p.visible_bricks_ptr.count, 1, write_idx);
            InterlockedAdd(p.compact_visible_ptr.count, 1, write_idx);
            p.compact_visible_ptr.indices[write_idx] = global_idx;
        }
    }
    else
    {
        render_image[pixel] = 0.0;
    }
}
