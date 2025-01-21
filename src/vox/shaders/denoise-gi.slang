#include "shared.inl"
[[vk::push_constant]]
const DenoisePush p;
// À-Trous filter weights based on a B3 spline (1/16, 1/4, 3/8, 1/4, 1/16)
static const float kernel[25] = {
    1.0f / 256.0f, 1.0f / 64.0f, 3.0f / 128.0f, 1.0f / 64.0f, 1.0f / 256.0f,
    1.0f / 64.0f, 1.0f / 16.0f, 3.0f / 32.0f, 1.0f / 16.0f, 1.0f / 64.0f,
    3.0f / 128.0f, 3.0f / 32.0f, 9.0f / 64.0f, 3.0f / 32.0f, 3.0f / 128.0f,
    1.0f / 64.0f, 1.0f / 16.0f, 3.0f / 32.0f, 1.0f / 16.0f, 1.0f / 64.0f,
    1.0f / 256.0f, 1.0f / 64.0f, 3.0f / 128.0f, 1.0f / 64.0f, 1.0f / 256.0f
};
// Edge-stopping function
float edge_stopping(float value, float sigma)
{
    return exp(-value / (sigma * sigma));
}
[numthreads(16, 16, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    uint2 pixel = pixel_i.xy;
    if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
        return;
    RWTexture2D<float3> image_indirect = RWTexture2D<float3>::get(p.gbuffer.indirect);
    RWTexture2D<float> image_depth = RWTexture2D<float>::get(p.gbuffer.depth);
    RWTexture2D<float3> image_normal = RWTexture2D<float3>::get(p.gbuffer.normal);
    // Get center pixel values
    float3 center_color = image_indirect[pixel];
    float center_depth = image_depth[pixel];
    float3 center_normal = image_normal[pixel] * 2.0 - 1.0;
    // Filter parameters
    const float sigma_color = 1000;   // Color edge-stopping parameter
    const float sigma_normal = 0.01f; // Normal edge-stopping parameter
    const float sigma_depth = 0.3f;   // Depth edge-stopping parameter
    float3 sum_color = float3(0, 0, 0);
    float sum_weights = 0.0f;
    // 5x5 filter kernel
    [unroll]
    for (int y = -2; y <= 2; y++)
    {
        [unroll]
        for (int x = -2; x <= 2; x++)
        {
            int2 sample_pos = int2(pixel) + int2(x, y) * (p.pass + 4);
            // Skip samples outside the image
            if (sample_pos.x < 0 || sample_pos.x >= p.frame_dim.x ||
                sample_pos.y < 0 || sample_pos.y >= p.frame_dim.y)
                continue;
            // Get sample values
            float3 sample_color = image_indirect[sample_pos];
            float sample_depth = image_depth[sample_pos];
            float3 sample_normal = image_normal[sample_pos] * 2.0 - 1.0;

            if (sample_depth == 0.0f)
                continue;

            // Compute weights based on color, normal, and depth differences
            float color_diff = length(sample_color.rgb - center_color.rgb);
            float normal_diff = 1.0f - dot(sample_normal.xyz, center_normal.xyz);
            float depth_diff = abs(sample_depth - center_depth);
            // Combine edge-stopping functions
            // float w_color = edge_stopping(color_diff, sigma_color);
            float w_color = 1.0f;
            float w_normal = edge_stopping(normal_diff, sigma_normal);
            float w_depth = edge_stopping(depth_diff, sigma_depth);
            // Final weight combines kernel weight with edge-stopping weights
            float kernel_weight = kernel[(y + 2) * 5 + (x + 2)];
            float weight = kernel_weight * w_color * w_normal * w_depth;
            sum_color += sample_color * weight;
            sum_weights += weight;
        }
    }
    // Write filtered result
    image_indirect[pixel] = sum_color / sum_weights;
}
