#include "shared.inl"

[[vk::push_constant]]
const ComputePush p;

// 3x3 kernel with similar Gaussian distribution
static const float kernel3x3[9] = {
    1.0f / 16.0f, 1.0f / 8.0f, 1.0f / 16.0f,
    1.0f / 8.0f, 1.0f / 4.0f, 1.0f / 8.0f,
    1.0f / 16.0f, 1.0f / 8.0f, 1.0f / 16.0f
};

// 5x5 kernel with similar Gaussian distribution
static const float kernel5x5[25] = {
    1.0f / 64.0f, 1.0f / 32.0f, 1.0f / 16.0f, 1.0f / 32.0f, 1.0f / 64.0f,
    1.0f / 32.0f, 1.0f / 16.0f, 1.0f / 8.0f, 1.0f / 16.0f, 1.0f / 32.0f,
    1.0f / 16.0f, 1.0f / 8.0f, 1.0f / 4.0f, 1.0f / 8.0f, 1.0f / 16.0f,
    1.0f / 32.0f, 1.0f / 16.0f, 1.0f / 8.0f, 1.0f / 16.0f, 1.0f / 32.0f,
    1.0f / 64.0f, 1.0f / 32.0f, 1.0f / 16.0f, 1.0f / 32.0f, 1.0f / 64.0f
};

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

float color_edge_stopping(float3 c1, float3 c2, float sigma)
{
    float3 diff = c1 - c2;
    float diff_sq = dot(diff, diff);
    return exp(-diff_sq / (sigma * sigma));
}

[numthreads(16, 16, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    uint2 pixel = pixel_i.xy;
    if (pixel.x >= p.screenSize.x || pixel.y >= p.screenSize.y)
        return;

    RWTexture2D<float3> image_original = RWTexture2D<float3>::get(p.gbuffer.ao);
    RWTexture2D<float3> image_denoise = RWTexture2D<float3>::get(p.gbuffer.aoDenoised);

    // image_denoise[pixel] = image_original[pixel];
    // return;

    RWTexture2D<float3> image_position = RWTexture2D<float3>::get(p.gbuffer.position);
    RWTexture2D<float3> image_normal = RWTexture2D<float3>::get(p.gbuffer.normal);
    RWTexture2D<float> image_depth = RWTexture2D<float>::get(p.gbuffer.depth);
    RWTexture2D<uint2> voxelIDs = RWTexture2D<uint2>::get(p.gbuffer.voxelIDs);

    uint2 voxelID = voxelIDs[pixel * LIGHTING_DOWNSAMPLE];
    float3 center_pos = image_position[pixel * LIGHTING_DOWNSAMPLE];

    // Skip denoising for sky pixels
    if (all(center_pos == float3(0, 0, 0)))
        return;

    if (p.passNum > 2)
        return;

    float3 center_color;
    if (p.passNum <= 1)
    {
        center_color = image_original[pixel];
    }
    else
    {
        center_color = image_denoise[pixel];
    }
    float3 center_normal = image_normal[pixel * LIGHTING_DOWNSAMPLE];
    float center_depth = image_depth[pixel * LIGHTING_DOWNSAMPLE];

    const float sigma_geom = 0.01f;
    const float sigma_normal = 0.1f;
    const float sigma_color = 0.4f;

    // Second pass - apply denoising with firefly removal
    float3 sum_color = float3(0, 0, 0);
    float sum_weights = 0.0f;

    [unroll]
    for (int y = -2; y <= 2; y++)
    {
        [unroll]
        for (int x = -2; x <= 2; x++)
        {
            int2 offset_pos = int2(pixel) + int2(x, y) * p.passNum;

            if (offset_pos.x < 0 || offset_pos.x >= p.screenSize.x ||
                offset_pos.y < 0 || offset_pos.y >= p.screenSize.y)
                continue;

            float3 sample_worldpos = image_position[offset_pos * LIGHTING_DOWNSAMPLE];
            float3 sample_color;

            if (p.passNum <= 1)
            {
                sample_color = image_original[offset_pos];
            }
            else
            {
                sample_color = image_denoise[offset_pos];
            }

            float3 sample_normal = image_normal[offset_pos * LIGHTING_DOWNSAMPLE];

            // Skip sky samples
            if (all(sample_normal == float3(0, 0, 0)))
                continue;

            float kernel_weight = kernel5x5[(y + 2) * 5 + (x + 2)];
            float geom_weight = geometry_edge_stopping(center_pos, sample_worldpos, center_normal, sigma_geom);
            float normal_weight = normal_edge_stopping(center_normal, sample_normal, sigma_normal);
            float color_weight = color_edge_stopping(center_color, sample_color, sigma_color);

            float weight = kernel_weight * geom_weight * normal_weight;

            // if (p.passNum > 0)
            // {
            //     weight *= color_weight;
            // }

            sum_color += sample_color * weight;
            sum_weights += weight;
        }
    }

    var final = sum_weights > 1e-6f ? sum_color / sum_weights : center_color;

    image_denoise[pixel] = final;

    // if (p.pass <= 1)
    // image_indirect[pixel].xyz = final;
}