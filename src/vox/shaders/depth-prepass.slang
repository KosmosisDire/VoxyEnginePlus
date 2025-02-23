#include "atmosphere.slang"
#include "raycast.slang"

[numthreads(8, 8, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    // return;
    // RWTexture2D<float> depth_tex = RWTexture2D<float>::get(p.gbuffer.depthHalfRes);

    // uint2 pixel = pixel_i.xy;
    // if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
    //     return;

    // // Ray setup
    // float3 rayOrigin, rayDir;
    // getRayFromPixel(pixel, p.frame_dim / 2, p.state_ptr.camera, rayOrigin, rayDir);

    // TraceResult trace = traceVoxelRay(rayOrigin, rayDir);

    // if (trace.hit)
    // {
    //     depth_tex[pixel] = trace.distance;
    // }
    // else
    // {
    //     depth_tex[pixel] = 0.0;
    // }
}
