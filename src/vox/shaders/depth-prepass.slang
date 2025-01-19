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
    }
    else
    {
        render_image[pixel] = 0.0;
    }
}
