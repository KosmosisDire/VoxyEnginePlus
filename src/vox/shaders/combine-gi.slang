#include "shared.inl"
#include "voxel-hashmap.slang"

[[vk::push_constant]]
const ComputePush p;

[numthreads(8, 8, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    RWTexture2D<float3> indirect = RWTexture2D<float3>::get(p.gbuffer.indirectDenoised);
    RWTexture2D<float3> indirectPerVoxPass1 = RWTexture2D<float3>::get(p.gbuffer.indirectPerVoxelPass1);
    RWTexture2D<float3> indirectPerVoxPass2 = RWTexture2D<float3>::get(p.gbuffer.indirectPerVoxelPass2);

    RWTexture2D<float> image_ssao = RWTexture2D<float>::get(p.gbuffer.ssao);
    RWTexture2D<float3> color = RWTexture2D<float3>::get(p.gbuffer.color);
    RWTexture2D<int2> voxelIDs = RWTexture2D<int2>::get(p.gbuffer.voxelIDs);
    RWTexture2D<uint> voxelFaceIDs = RWTexture2D<uint>::get(p.gbuffer.voxelFaceIDs);
    RWTexture2D<float2> voxelUVs = RWTexture2D<float2>::get(p.gbuffer.voxelUVs);
    RWTexture2D<float3> position = RWTexture2D<float3>::get(p.gbuffer.position);
    RWTexture2D<float> depth = RWTexture2D<float>::get(p.gbuffer.depthHalfRes);
    RWTexture2D<int3> normals = RWTexture2D<int3>::get(p.gbuffer.normal);
    RWTexture2D<float2> motion = RWTexture2D<float2>::get(p.gbuffer.motion);

    // find boundaries from adjacent pixels based on voxelids
    uint2 pixel = pixel_i.xy;
    int2 voxelID = voxelIDs[pixel * 2];
    uint voxelFaceID = voxelFaceIDs[pixel * 2];
    float2 voxelUV = voxelUVs[pixel * 2];
    float depthValue = depth[pixel];
    int3 normal = normals[pixel * 2];
    float3 pos = position[pixel * 2];
    float2 motionVector = motion[pixel * 2];

    if (p.pass == 1)
    {
        float3 avgColor = indirect[pixel];

        int avgCount = 1;
        int2 offsetVoxelID = voxelIDs[(pixel + int2(-1, 0)) * 2];
        uint offsetFaceID = voxelFaceIDs[(pixel + int2(-1, 0)) * 2];
        if (offsetVoxelID.x != voxelID.x || offsetFaceID != voxelFaceID)
        {
            offsetVoxelID = voxelIDs[(pixel + int2(avgCount, 0)) * 2];
            offsetFaceID = voxelFaceIDs[(pixel + int2(avgCount, 0)) * 2];
            while (offsetVoxelID.x == voxelID.x && offsetFaceID == voxelFaceID && avgCount < 1000)
            {
                float3 sampleColor = indirect[(pixel + int2(avgCount, 0))];
                avgColor += sampleColor;

                avgCount++;
                offsetVoxelID = voxelIDs[(pixel + int2(avgCount, 0)) * 2];
                offsetFaceID = voxelFaceIDs[(pixel + int2(avgCount, 0)) * 2];
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
        int2 offsetVoxelID = voxelIDs[(pixel + int2(0, -1)) * 2];
        uint offsetFaceID = voxelFaceIDs[(pixel + int2(0, -1)) * 2];
        if (offsetVoxelID.x != voxelID.x || offsetFaceID != voxelFaceID)
        {
            offsetVoxelID = voxelIDs[(pixel + int2(0, avgCount)) * 2];
            offsetFaceID = voxelFaceIDs[(pixel + int2(0, avgCount)) * 2];
            while (offsetVoxelID.x == voxelID.x && offsetFaceID == voxelFaceID && avgCount < 1000)
            {
                float3 sampleColor = indirectPerVoxPass1[(pixel + int2(0, avgCount))];
                avgColor += sampleColor;

                avgCount++;
                offsetVoxelID = voxelIDs[(pixel + int2(0, avgCount)) * 2];
                offsetFaceID = voxelFaceIDs[(pixel + int2(0, avgCount)) * 2];
            }

            for (int i = 0; i < avgCount; i++)
            {
                indirectPerVoxPass2[(pixel + int2(0, i))] = avgColor / avgCount;
            }
        }
    }

    if (p.pass == 3)
    {
        int2 offsetVoxID = voxelIDs[(pixel + int2(-1, -1)) * 2];
        int2 offsetVoxID2 = voxelIDs[(pixel + int2(1, 1)) * 2];
        int offsetFaceID = voxelFaceIDs[(pixel + int2(-1, -1)) * 2];
        int offsetFaceID2 = voxelFaceIDs[(pixel + int2(1, 1)) * 2];

        if (any(offsetVoxID != voxelID) || any(offsetVoxID2 != voxelID) || offsetFaceID != voxelFaceID || offsetFaceID2 != voxelFaceID)
        {
            float3 finalColor = indirectPerVoxPass2[pixel];

            uint slot = get_hashtable_index(p.voxel_hashmap_ptr, voxelID.x, voxelFaceID);
            uint lastTempFrames = 0;
            float3 lastColor = float3(0, 0, 0);

            if (slot != EMPTY_KEY)
            {
                var entry = p.voxel_hashmap_ptr.entries[slot];
                lastTempFrames = entry.historyCount;
                lastColor = entry.lighting;
            }

            float blendFactor = 1.0 / (lastTempFrames + 1);
            blendFactor = clamp(blendFactor, 0.01, 0.99);
            finalColor = lastColor * (1 - blendFactor) + finalColor * (blendFactor);

            voxel_hashtable_insert(p.voxel_hashmap_ptr, voxelID.x, voxelFaceID, finalColor, lastTempFrames + 1, p.state_ptr.frame);
        }
    }
}
