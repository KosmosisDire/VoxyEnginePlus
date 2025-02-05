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

[numthreads(8, 8, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    RWTexture2D<float3> indirect = RWTexture2D<float3>::get(p.gbuffer.indirectDenoised);
    RWTexture2D<float3> indirectPerVoxPass1 = RWTexture2D<float3>::get(p.gbuffer.indirectPerVoxelPass1);
    RWTexture2D<float3> indirectPerVoxPass2 = RWTexture2D<float3>::get(p.gbuffer.indirectPerVoxelPass2);

    RWTexture2D<float> image_ssao = RWTexture2D<float>::get(p.gbuffer.ssao);
    RWTexture2D<float3> color = RWTexture2D<float3>::get(p.gbuffer.color);
    RWTexture2D<int2> voxelIDs = RWTexture2D<int2>::get(p.gbuffer.voxelIDs);
    RWTexture2D<float2> voxelUVs = RWTexture2D<float2>::get(p.gbuffer.voxelUVs);
    RWTexture2D<float> depth = RWTexture2D<float>::get(p.gbuffer.depthHalfRes);
    RWTexture2D<int3> normals = RWTexture2D<int3>::get(p.gbuffer.normal);

    // find boundaries from adjacent pixels based on voxelids
    uint2 pixel = pixel_i.xy;
    int2 voxelID = voxelIDs[pixel * 2];
    float2 voxelUV = voxelUVs[pixel * 2];
    float depthValue = depth[pixel];
    int3 normal = normals[pixel * 2];

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
        avgColor = lerp(avgColor * image_ssao[pixel], avgColor, clamp(dot(avgColor, float3(0.2126, 0.7152, 0.0722)) * 0.5, 0, 1));

        int avgCount = 1;
        int2 offsetVoxID = voxelIDs[(pixel + int2(-1, 0)) * 2];
        if (offsetVoxID.x != voxelID.x || offsetVoxID.y != voxelID.y)
        {
            offsetVoxID = voxelIDs[(pixel + int2(avgCount, 0)) * 2];
            while (offsetVoxID.x == voxelID.x && offsetVoxID.y == voxelID.y && avgCount < 1000)
            {
                float3 sampleColor = indirect[(pixel + int2(avgCount, 0))];
                sampleColor = lerp(sampleColor * image_ssao[pixel], sampleColor, clamp(dot(sampleColor, float3(0.2126, 0.7152, 0.0722)) * 0.5, 0, 1));
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

    if (p.pass == 3)
    {
        float3 avgColor = indirectPerVoxPass2[pixel];

        int avgCount = 1;
        int2 offsetVoxID = voxelIDs[(pixel + int2(-1, 0)) * 2];
        if (offsetVoxID.x != voxelID.x || offsetVoxID.y != voxelID.y)
        {
            offsetVoxID = voxelIDs[(pixel + int2(avgCount, 0)) * 2];
            while (offsetVoxID.x == voxelID.x && offsetVoxID.y == voxelID.y && avgCount < 1000)
            {
                float3 sampleColor = indirectPerVoxPass2[(pixel + int2(avgCount, 0))];
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

    if (p.pass == 4)
    {
        float3 avgColor = indirectPerVoxPass2[pixel];
        int avgCount = 0;
        int2 offsetVoxID = voxelIDs[(pixel + int2(-1, -1)) * 2];
        int2 offsetVoxID2 = voxelIDs[(pixel + int2(1, 1)) * 2];

        float3 rayOrigin, rayDir;
        getRayFromPixel(pixel, p.frame_dim, p.state_ptr.camera, rayOrigin, rayDir);
        float roughArea = depthValue * 0.03 * (dot(normal, rayDir * 0.5 + 0.5) + 2);

        if (offsetVoxID.x != voxelID.x || offsetVoxID.y != voxelID.y || offsetVoxID2.x != voxelID.x || offsetVoxID2.y != voxelID.y)
        {
            float3 finalColor = indirectPerVoxPass1[pixel];

            uint slot = get_hashtable_index(p.past_voxel_hashmap_ptr, voxelID.x, voxelID.y);
            uint lastTempFrames = p.past_voxel_hashmap_ptr.temporalFrames[slot];
            float3 lastColor = p.past_voxel_hashmap_ptr.indirectLight[slot];

            float blendFactor = 1.0 / (lastTempFrames + 1);
            blendFactor = clamp(blendFactor, 0.02, 0.99);
            finalColor = lastColor * (1 - blendFactor) + finalColor * (blendFactor);

            voxel_hashtable_insert(p.voxel_hashmap_ptr, voxelID.x, voxelID.y, finalColor, lastTempFrames + 1);
        }
    }

    // return;
    // uint2 pixel = pixel_i.xy;
    // if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
    //     return;

    // RWTexture2D<float3> indirect = RWTexture2D<float3>::get(p.gbuffer.indirectDenoised);
    // RWTexture2D<int2> voxelIDs = RWTexture2D<int2>::get(p.gbuffer.voxelIDs);
    // RWTexture2D<float> image_ssao = RWTexture2D<float>::get(p.gbuffer.ssao);
    // int2 voxelID = voxelIDs[pixel];

    // if (voxelID.x == EMPTY_KEY && voxelID.y == EMPTY_KEY)
    // {
    //     return;
    // }

    // uint slot = get_hashtable_index(p.past_voxel_hashmap_ptr, voxelID.x, voxelID.y);

    // var color = indirect[pixel / 2].xyz;
    // // color = lerp(color * image_ssao[pixel / 2], color, clamp(dot(color, float3(0.2126, 0.7152, 0.0722)) * 0.5, 0, 1));
    // color *= image_ssao[pixel / 2];

    // if (slot == EMPTY_KEY)
    // {
    //     voxel_hashtable_insert(p.voxel_hashmap_ptr, voxelID.x, voxelID.y, color, 0);
    //     voxel_hashtable_insert(p.voxel_hashmap_ptr, voxelID.x, 0, color, 0); // also insert one on 0
    //     return;
    // }

    // float lastHits = p.past_voxel_hashmap_ptr.hitCounts[slot];
    // float3 lastColor = (p.past_voxel_hashmap_ptr.indirectLight[slot] / max(lastHits, 1));
    // int lastTempFrames = p.past_voxel_hashmap_ptr.temporalFrames[slot];

    // float variance = 0.0;
    // if (lastTempFrames > 1)
    // {
    //     float3 diff = color - lastColor;
    //     variance = dot(diff, diff) / (lastTempFrames - 1);
    // }

    // float lastLuminance = dot(lastColor, float3(0.2126, 0.7152, 0.0722));
    // float blendFactor = 1.0 / (lastTempFrames) * (1 - variance);
    // blendFactor = clamp(blendFactor, 0.01, 0.9);
    // float3 thisFrameIndirect = lastColor * (1 - blendFactor) + color * (blendFactor);
    // // indirect[pixel / 2] = thisFrameIndirect;

    // voxel_hashtable_insert(p.voxel_hashmap_ptr, voxelID.x, voxelID.y, thisFrameIndirect, lastTempFrames + 1);
    // voxel_hashtable_insert(p.voxel_hashmap_ptr, voxelID.x, 0, thisFrameIndirect, lastTempFrames + 1); // also insert one on 0
}
