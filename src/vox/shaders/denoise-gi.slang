#include "shared.inl"

[[vk::push_constant]]
const DenoisePush p;

// 5x5 kernel remains the same
static const float kernel5x5[25] = {
    1.0f / 256.0f, 1.0f / 64.0f, 3.0f / 128.0f, 1.0f / 64.0f, 1.0f / 256.0f,
    1.0f / 64.0f, 1.0f / 16.0f, 3.0f / 32.0f, 1.0f / 16.0f, 1.0f / 64.0f,
    3.0f / 128.0f, 3.0f / 32.0f, 9.0f / 64.0f, 3.0f / 32.0f, 3.0f / 128.0f,
    1.0f / 64.0f, 1.0f / 16.0f, 3.0f / 32.0f, 1.0f / 16.0f, 1.0f / 64.0f,
    1.0f / 256.0f, 1.0f / 64.0f, 3.0f / 128.0f, 1.0f / 64.0f, 1.0f / 256.0f
};

// 3x3 kernel with similar Gaussian distribution
static const float kernel3x3[9] = {
    1.0f / 16.0f, 1.0f / 8.0f, 1.0f / 16.0f,
    1.0f / 8.0f, 1.0f / 4.0f, 1.0f / 8.0f,
    1.0f / 16.0f, 1.0f / 8.0f, 1.0f / 16.0f
};

// Define as compile time constant for better optimization
#define USE_KERNEL_SIZE_3 1

float3 convert_normal(float3 normal)
{
    return normal * 2.0f - 1.0f;
}

float geometry_edge_stopping(float3 pos1, float3 pos2, float3 normal, float sigma)
{
    float3 pos_diff = pos2 - pos1;
    float diff_along_normal = abs(dot(pos_diff, normal));
    return exp(-diff_along_normal / sigma);
}

float normal_edge_stopping(float3 n1, float3 n2, float sigma)
{
    float normal_similarity = dot(n1, n2);
    float diff = 1.0f - normal_similarity;
    return exp(-diff / (sigma * sigma));
}

[numthreads(16, 16, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    // return;
    uint2 pixel = pixel_i.xy;
    if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
        return;

    RWTexture2D<float3> image_indirect = RWTexture2D<float3>::get(p.gbuffer.indirect);
    RWTexture2D<float3> image_position = RWTexture2D<float3>::get(p.gbuffer.position);
    RWTexture2D<float3> image_normal = RWTexture2D<float3>::get(p.gbuffer.normal);
    RWTexture2D<float> image_depth = RWTexture2D<float>::get(p.gbuffer.depthHalfRes);

    float3 center_pos = image_position[pixel * 2];

    // Skip denoising for sky pixels
    if (all(center_pos == float3(0, 0, 0)))
        return;

    float3 center_color = image_indirect[pixel];
    float3 center_normal = convert_normal(image_normal[pixel * 2]);
    float center_depth = image_depth[pixel];

    const float sigma_geom = 0.1f;
    const float sigma_normal = 0.1f;

    float3 sum_color = float3(0, 0, 0);
    float sum_weights = 0.0f;

#if USE_KERNEL_SIZE_3
    const int kernel_radius = 1;
    [unroll]
    for (int y = -kernel_radius; y <= kernel_radius; y++)
    {
        [unroll]
        for (int x = -kernel_radius; x <= kernel_radius; x++)
        {
            int2 offset_pos = int2(pixel) + int2(x, y) * (center_depth < 10 ? p.pass * 1 : p.pass);

            if (offset_pos.x < 0 || offset_pos.x >= p.frame_dim.x ||
                offset_pos.y < 0 || offset_pos.y >= p.frame_dim.y)
                continue;

            float3 sample_worldpos = image_position[offset_pos * 2];

            // Skip sky samples
            if (all(sample_worldpos == float3(0, 0, 0)))
                continue;

            float3 sample_color = image_indirect[offset_pos];
            float3 sample_normal = convert_normal(image_normal[offset_pos * 2]);

            float kernel_weight = kernel3x3[(y + 1) * 3 + (x + 1)];
            float geom_weight = geometry_edge_stopping(center_pos, sample_worldpos, center_normal, sigma_geom);
            float normal_weight = normal_edge_stopping(center_normal, sample_normal, sigma_normal);
            float weight = kernel_weight * geom_weight * normal_weight;

            sum_color += sample_color * weight;
            sum_weights += weight;
        }
    }
#else
    const int kernel_radius = 2;
    [unroll]
    for (int y = -kernel_radius; y <= kernel_radius; y++)
    {
        [unroll]
        for (int x = -kernel_radius; x <= kernel_radius; x++)
        {
            int2 offset_pos = int2(pixel) + int2(x, y) * (center_depth < 10 ? p.pass * 1 : p.pass);

            if (offset_pos.x < 0 || offset_pos.x >= p.frame_dim.x ||
                offset_pos.y < 0 || offset_pos.y >= p.frame_dim.y)
                continue;

            float3 sample_worldpos = image_position[offset_pos * 2];

            // Skip sky samples
            if (all(sample_worldpos == float3(0, 0, 0)))
                continue;

            float3 sample_color = image_indirect[offset_pos];
            float3 sample_normal = convert_normal(image_normal[offset_pos * 2]);

            float kernel_weight = kernel5x5[(y + 2) * 5 + (x + 2)];
            float geom_weight = geometry_edge_stopping(center_pos, sample_worldpos, center_normal, sigma_geom);
            float normal_weight = normal_edge_stopping(center_normal, sample_normal, sigma_normal);
            float weight = kernel_weight * geom_weight * normal_weight;

            sum_color += sample_color * weight;
            sum_weights += weight;
        }
    }
#endif

    var final = sum_weights > 1e-6f ? sum_color / sum_weights : center_color;
    image_indirect[pixel] = final;
}
