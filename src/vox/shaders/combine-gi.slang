#include "shared.inl"
#include "voxel-hashmap.slang"

[[vk::push_constant]]
const ComputePush p;

void getRayFromPixel(float2 pixel, uint2 dimensions, CameraData camera, out float3 rayOrigin, out float3 rayDir)
{
    // Convert pixel to NDC space (-1 to 1)
    float2 uv = (pixel + 0.5f) / float2(dimensions);
    float2 ndc = uv * 2.0f - 1.0f;

    // Inverse projection to get view space position
    float4 viewSpace = mul(camera.invViewProj, float4(ndc.x, ndc.y, 1.0f, 1.0f));
    viewSpace /= viewSpace.w;

    // Get ray direction in world space
    rayOrigin = camera.position;
    rayDir = normalize(viewSpace.xyz - camera.position);
}

void getBilinearWeights(float2 pos, out int2 baseCoord, out float2 weights)
{
    baseCoord = int2(floor(pos));
    weights = frac(pos);
}

float3 sampleBilinear(RWTexture2D<float3> img, float2 pos, float2 dimensions)
{
    int2 baseCoord;
    float2 weights;
    getBilinearWeights(pos, baseCoord, weights);

    // Clamp coordinates to texture bounds
    int2 c00 = clamp(baseCoord, int2(0, 0), int2(dimensions) - 1);
    int2 c10 = clamp(int2(baseCoord.x + 1, baseCoord.y), int2(0, 0), int2(dimensions) - 1);
    int2 c01 = clamp(int2(baseCoord.x, baseCoord.y + 1), int2(0, 0), int2(dimensions) - 1);
    int2 c11 = clamp(baseCoord + int2(1, 1), int2(0, 0), int2(dimensions) - 1);

    // Sample four nearest pixels
    float3 s00 = img[c00];
    float3 s10 = img[c10];
    float3 s01 = img[c01];
    float3 s11 = img[c11];

    // Bilinear blend
    float3 s0 = lerp(s00, s10, weights.x);
    float3 s1 = lerp(s01, s11, weights.x);
    return lerp(s0, s1, weights.y);
}

[numthreads(8, 8, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    RWTexture2D<float3> indirect = RWTexture2D<float3>::get(p.gbuffer.indirectDenoised);
    RWTexture2D<float3> indirectPerVoxPass1 = RWTexture2D<float3>::get(p.gbuffer.indirectPerVoxelPass1);
    RWTexture2D<float3> indirectPerVoxPass2 = RWTexture2D<float3>::get(p.gbuffer.indirectPerVoxelPass2);

    RWTexture2D<float> image_ssao = RWTexture2D<float>::get(p.gbuffer.ssao);
    RWTexture2D<float3> color = RWTexture2D<float3>::get(p.gbuffer.color);
    RWTexture2D<int2> voxelIDs = RWTexture2D<int2>::get(p.gbuffer.voxelIDs);
    RWTexture2D<int2> voxelIDsLast = RWTexture2D<int2>::get(p.gbuffer.voxelIDsLast);
    RWTexture2D<float2> voxelUVs = RWTexture2D<float2>::get(p.gbuffer.voxelUVs);
    RWTexture2D<float3> position = RWTexture2D<float3>::get(p.gbuffer.position);
    RWTexture2D<float> depth = RWTexture2D<float>::get(p.gbuffer.depthHalfRes);
    RWTexture2D<int3> normals = RWTexture2D<int3>::get(p.gbuffer.normal);
    RWTexture2D<float2> motion = RWTexture2D<float2>::get(p.gbuffer.motion);

    // find boundaries from adjacent pixels based on voxelids
    uint2 pixel = pixel_i.xy;
    int2 voxelID = voxelIDs[pixel * 2];
    float2 voxelUV = voxelUVs[pixel * 2];
    float depthValue = depth[pixel];
    int3 normal = normals[pixel * 2];
    float3 pos = position[pixel * 2];
    float2 motionVector = motion[pixel * 2];

    // image_ssao[pixel] = 0.0;
    // int2 offsetVoxID = voxelIDs[(pixel + int2(-1, 0)) * 2];
    // if (offsetVoxID.x != voxelID.x || offsetVoxID.y != voxelID.y)
    // {
    //     image_ssao[pixel] = 1.0;
    // }

    int2 jitter = int2(p.state_ptr.frame % 2, (p.state_ptr.frame + 1) % 2);

    if (p.pass == 1)
    {
        float3 avgColor = indirect[pixel];
        // avgColor = lerp(avgColor * image_ssao[pixel], avgColor, clamp(dot(avgColor, float3(0.2126, 0.7152, 0.0722)) * 0.5, 0, 1));

        int avgCount = 1;
        int2 offsetVoxID = voxelIDs[(pixel + int2(-1, 0)) * 2];
        if (offsetVoxID.x != voxelID.x || offsetVoxID.y != voxelID.y)
        {
            offsetVoxID = voxelIDs[(pixel + int2(avgCount, 0)) * 2];
            while (offsetVoxID.x == voxelID.x && offsetVoxID.y == voxelID.y && avgCount < 1000)
            {
                float3 sampleColor = indirect[(pixel + int2(avgCount, 0))];
                // sampleColor = lerp(sampleColor * image_ssao[pixel], sampleColor, clamp(dot(sampleColor, float3(0.2126, 0.7152, 0.0722)) * 0.5, 0, 1));
                avgColor += sampleColor;

                avgCount++;
                offsetVoxID = voxelIDs[(pixel + int2(avgCount, 0)) * 2];
            }

            for (int i = 0; i < avgCount; i++)
            {
                indirectPerVoxPass1[(pixel + int2(i, 0))] = avgColor / avgCount;
            }
        }
    }

    if (p.pass == 2)
    {
        float3 avgColor = indirectPerVoxPass1[pixel];
        int avgCount = 1;
        int2 offsetVoxID = voxelIDs[(pixel + int2(0, -1)) * 2];
        if (offsetVoxID.x != voxelID.x || offsetVoxID.y != voxelID.y)
        {
            offsetVoxID = voxelIDs[(pixel + int2(0, avgCount)) * 2];
            while (offsetVoxID.x == voxelID.x && offsetVoxID.y == voxelID.y && avgCount < 1000)
            {
                avgColor += indirectPerVoxPass1[(pixel + int2(0, avgCount))];

                avgCount++;
                offsetVoxID = voxelIDs[(pixel + int2(0, avgCount)) * 2];
            }

            for (int i = 0; i < avgCount; i++)
            {
                indirectPerVoxPass2[(pixel + int2(0, i))] = avgColor / avgCount;
            }
        }
    }

    // if (p.pass == 3)
    // {
    //     float3 avgColor = indirectPerVoxPass2[pixel];

    //     int avgCount = 1;
    //     int2 offsetVoxID = voxelIDs[(pixel + int2(-1, 0)) * 2];
    //     if (offsetVoxID.x != voxelID.x || offsetVoxID.y != voxelID.y)
    //     {
    //         offsetVoxID = voxelIDs[(pixel + int2(avgCount, 0)) * 2];
    //         while (offsetVoxID.x == voxelID.x && offsetVoxID.y == voxelID.y && avgCount < 1000)
    //         {
    //             float3 sampleColor = indirectPerVoxPass2[(pixel + int2(avgCount, 0))];
    //             avgColor += sampleColor;

    //             avgCount++;
    //             offsetVoxID = voxelIDs[(pixel + int2(avgCount, 0)) * 2];
    //         }

    //         for (int i = 0; i < avgCount; i++)
    //         {
    //             indirectPerVoxPass1[(pixel + int2(i, 0))] = avgColor / avgCount;
    //         }
    //     }
    // }

    if (p.pass == 4)
    {
        int2 offsetVoxID = voxelIDs[(pixel + int2(-1, -1) * jitter) * 2];
        int2 offsetVoxID2 = voxelIDs[(pixel + int2(1, 1)) * 2];

        float3 rayOrigin, rayDir;
        getRayFromPixel(pixel, p.frame_dim, p.state_ptr.camera, rayOrigin, rayDir);
        float roughArea = depthValue * 0.03 * (dot(normal, rayDir * 0.5 + 0.5) + 2);

        if (offsetVoxID.x != voxelID.x || offsetVoxID.y != voxelID.y || offsetVoxID2.x != voxelID.x || offsetVoxID2.y != voxelID.y)
        {
            float3 finalColor = indirectPerVoxPass2[pixel];

            uint slot = get_hashtable_index(p.past_voxel_hashmap_ptr, voxelID.x, voxelID.y);
            uint lastTempFrames = 0;
            float3 lastColor = float3(0, 0, 0);

            // if last slot is empty then we need to try and find a reprojection of the last frame using a different voxel id (like the one from the previous frame)
            if (slot != EMPTY_KEY)
            {
                lastTempFrames = p.past_voxel_hashmap_ptr.temporalFrames[slot];
                lastColor = p.past_voxel_hashmap_ptr.indirectLight[slot];
            }

            float blendFactor = 1.0 / (lastTempFrames + 1);
            blendFactor = clamp(blendFactor, 0.01, 0.99);
            finalColor = lastColor * (1 - blendFactor) + finalColor * (blendFactor);

            voxel_hashtable_insert(p.voxel_hashmap_ptr, voxelID.x, voxelID.y, finalColor, lastTempFrames + 1, p.state_ptr.frame);
        }
    }
}
