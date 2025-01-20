#include "shared.inl"

[[vk::push_constant]]
const ComputePush p;

[numthreads(8, 8, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    uint2 pixel = pixel_i.xy;
    if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
        return;

    RWTexture2D<float4> image_indirect = RWTexture2D<float4>::get(p.gbuffer.indirect);
    RWTexture2D<float> image_depth = RWTexture2D<float>::get(p.gbuffer.indirect);
    RWTexture2D<float4> image_normal = RWTexture2D<float4>::get(p.gbuffer.indirect);

    // distance along camera ray
    float depth = image_depth[pixel];
    float3 normal = image_normal[pixel].xyz;

    // apply blur
    // float3 color = float3(0.0);
    // float3 weights = float3(0.0);
    // for (int y = -1; y <= 1; y++)
    // {
    //     for (int x = -1; x <= 1; x++)
    //     {
    //         uint2 offset = pixel + uint2(x, y);
    //         if (offset.x < p.frame_dim.x && offset.y < p.frame_dim.y)
    //         {
    //             float3 sample = image_indirect[offset].xyz;
    //             float weight = 1.0;
    //             color += sample * weight;
    //             weights += weight;
    //         }
    //     }
    // }

    // image_indirect[pixel] = float4(color / weights, 1.0);
}
