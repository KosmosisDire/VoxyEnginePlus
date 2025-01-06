#include <shared.inl>

[[vk::push_constant]]
const ComputePush p;

// get a single bit from an array of 64 bit integers
bool getOccupancyBit(int index)
{
    int arrayIndex = index / 64;
    int bitIndex = index % 64;
    return (p.occupancy.occupancy[arrayIndex] & (1ull << bitIndex)) != 0;
}

[numthreads(8, 8, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    RWTexture2D<float4> render_image = RWTexture2D<float4>::get(p.image);
    if (pixel_i.x >= p.frame_dim.x || pixel_i.y >= p.frame_dim.y)
        return;

    // Clear pixel
    render_image[pixel_i.xy] = p.settings.backgroundColor * (int)getOccupancyBit(pixel_i.x);
}
