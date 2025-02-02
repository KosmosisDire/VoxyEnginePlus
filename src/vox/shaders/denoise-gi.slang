#include "shared.inl"
#include "voxel-hashmap.slang"

[[vk::push_constant]]
const DenoisePush p;

// 3x3 kernel with similar Gaussian distribution
static const float kernel3x3[9] = {
    1.0f / 16.0f, 1.0f / 8.0f, 1.0f / 16.0f,
    1.0f / 8.0f, 1.0f / 4.0f, 1.0f / 8.0f,
    1.0f / 16.0f, 1.0f / 8.0f, 1.0f / 16.0f
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
    if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
        return;

    RWTexture2D<float3> image_indirectDenoise = RWTexture2D<float3>::get(p.gbuffer.indirectDenoised);
    RWTexture2D<float3> image_indirect = RWTexture2D<float3>::get(p.gbuffer.indirect);
    // image_indirectDenoise[pixel] = image_indirect[pixel];
    // return;

    RWTexture2D<float3> image_position = RWTexture2D<float3>::get(p.gbuffer.position);
    RWTexture2D<int3> image_normal = RWTexture2D<int3>::get(p.gbuffer.normal);
    RWTexture2D<float> image_depth = RWTexture2D<float>::get(p.gbuffer.depthHalfRes);
    RWTexture2D<uint2> voxelIDs = RWTexture2D<uint2>::get(p.gbuffer.voxelIDs);

    uint2 voxelID = voxelIDs[pixel * 2];
    float3 center_pos = image_position[pixel * 2];
    RWTexture2D<float3> read_texture = p.pass == 1 ? image_indirect : image_indirectDenoise;

    // Skip denoising for sky pixels
    if (all(center_pos == float3(0, 0, 0)))
        return;

    float3 center_color = read_texture[pixel];
    float3 center_normal = image_normal[pixel * 2];
    float center_depth = image_depth[pixel];

    const float sigma_geom = 0.01f;
    const float sigma_normal = 0.1f;
    const float sigma_color = 0.1f;

    // First pass - calculate mean and variance for firefly detection
    float3 mean_color = float3(0, 0, 0);
    float3 mean_sq_color = float3(0, 0, 0);
    float weight_sum = 0.0f;
    int valid_samples = 0;

    [unroll]
    for (int y = -1; y <= 1; y++)
    {
        [unroll]
        for (int x = -1; x <= 1; x++)
        {
            int2 offset_pos = int2(pixel) + int2(x, y) * p.pass;

            if (offset_pos.x < 0 || offset_pos.x >= p.frame_dim.x ||
                offset_pos.y < 0 || offset_pos.y >= p.frame_dim.y)
                continue;

            float3 sample_color = read_texture[offset_pos];
            float kernel_weight = kernel3x3[(y + 1) * 3 + (x + 1)];

            mean_color += sample_color * kernel_weight;
            mean_sq_color += sample_color * sample_color * kernel_weight;
            weight_sum += kernel_weight;
            valid_samples++;
        }
    }

    mean_color /= weight_sum;
    mean_sq_color /= weight_sum;
    float3 variance = max(mean_sq_color - mean_color * mean_color, 0.0f);
    float3 std_dev = sqrt(variance);

    // Second pass - apply denoising with firefly removal
    float3 sum_color = float3(0, 0, 0);
    float sum_weights = 0.0f;

    // Firefly removal threshold (adjust these constants to tune the effect)
    const float firefly_threshold = 300;

    [unroll]
    for (int y = -1; y <= 1; y++)
    {
        [unroll]
        for (int x = -1; x <= 1; x++)
        {
            int2 offset_pos = int2(pixel) + int2(x, y) * p.pass;

            if (offset_pos.x < 0 || offset_pos.x >= p.frame_dim.x ||
                offset_pos.y < 0 || offset_pos.y >= p.frame_dim.y)
                continue;

            float3 sample_worldpos = image_position[offset_pos * 2];
            float3 sample_color = read_texture[offset_pos];
            float3 sample_normal = image_normal[offset_pos * 2];

            // Skip sky samples
            if (all(sample_normal == float3(0, 0, 0)))
                continue;

            float kernel_weight = kernel3x3[(y + 1) * 3 + (x + 1)];
            float geom_weight = geometry_edge_stopping(center_pos, sample_worldpos, center_normal, sigma_geom);
            float normal_weight = normal_edge_stopping(center_normal, sample_normal, sigma_normal);
            float color_weight = color_edge_stopping(center_color, sample_color, sigma_color);
            // Firefly removal - reduce weight for samples that deviate too much from the mean
            float3 color_diff = abs(sample_color - mean_color);
            float3 threshold = firefly_threshold * std_dev;
            float firefly_weight = 1.0f;

            if (any(color_diff > threshold))
            {
                firefly_weight = exp(-dot(color_diff, color_diff) / dot(threshold, threshold));
            }

            float weight = kernel_weight * geom_weight * normal_weight * firefly_weight;

            sum_color += sample_color * weight;
            sum_weights += weight;
        }
    }

    var final = sum_weights > 1e-6f ? sum_color / sum_weights : center_color;

    image_indirectDenoise[pixel] = final;

    if (p.pass <= 3)
        image_indirect[pixel].xyz = final;
}
