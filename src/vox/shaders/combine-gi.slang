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

    RWTexture2D<float3> color = RWTexture2D<float3>::get(p.gbuffer.color);
    RWTexture2D<int2> voxelIDs = RWTexture2D<int2>::get(p.gbuffer.voxelIDs);
    RWTexture2D<uint> voxelFaceIDs = RWTexture2D<uint>::get(p.gbuffer.voxelFaceIDs);
    RWTexture2D<int3> normalTex = RWTexture2D<int3>::get(p.gbuffer.normal);

    uint2 pixel = pixel_i.xy;
    int2 voxelID = voxelIDs[pixel * LIGHTING_DOWNSAMPLE];
    uint voxelFaceID = voxelFaceIDs[pixel * LIGHTING_DOWNSAMPLE];

    if (p.pass == 1)
    {
        float3 avgColor = indirect[pixel];

        int avgCount = 1;
        int2 offsetVoxelID = voxelIDs[(pixel + int2(-1, 0)) * LIGHTING_DOWNSAMPLE];
        uint offsetFaceID = voxelFaceIDs[(pixel + int2(-1, 0)) * LIGHTING_DOWNSAMPLE];
        if (offsetVoxelID.x != voxelID.x || offsetFaceID != voxelFaceID)
        {
            offsetVoxelID = voxelIDs[(pixel + int2(avgCount, 0)) * LIGHTING_DOWNSAMPLE];
            offsetFaceID = voxelFaceIDs[(pixel + int2(avgCount, 0)) * LIGHTING_DOWNSAMPLE];
            while (offsetVoxelID.x == voxelID.x && offsetFaceID == voxelFaceID && avgCount < 1000)
            {
                float3 sampleColor = indirect[(pixel + int2(avgCount, 0))];
                avgColor += sampleColor;

                avgCount++;
                offsetVoxelID = voxelIDs[(pixel + int2(avgCount, 0)) * LIGHTING_DOWNSAMPLE];
                offsetFaceID = voxelFaceIDs[(pixel + int2(avgCount, 0)) * LIGHTING_DOWNSAMPLE];
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
        int2 offsetVoxelID = voxelIDs[(pixel + int2(0, -1)) * LIGHTING_DOWNSAMPLE];
        uint offsetFaceID = voxelFaceIDs[(pixel + int2(0, -1)) * LIGHTING_DOWNSAMPLE];
        if (offsetVoxelID.x != voxelID.x || offsetFaceID != voxelFaceID)
        {
            offsetVoxelID = voxelIDs[(pixel + int2(0, avgCount)) * LIGHTING_DOWNSAMPLE];
            offsetFaceID = voxelFaceIDs[(pixel + int2(0, avgCount)) * LIGHTING_DOWNSAMPLE];
            while (offsetVoxelID.x == voxelID.x && offsetFaceID == voxelFaceID && avgCount < 1000)
            {
                float3 sampleColor = indirectPerVoxPass1[(pixel + int2(0, avgCount))];
                avgColor += sampleColor;

                avgCount++;
                offsetVoxelID = voxelIDs[(pixel + int2(0, avgCount)) * LIGHTING_DOWNSAMPLE];
                offsetFaceID = voxelFaceIDs[(pixel + int2(0, avgCount)) * LIGHTING_DOWNSAMPLE];
            }

            for (int i = 0; i < avgCount; i++)
            {
                indirectPerVoxPass2[(pixel + int2(0, i))] = avgColor / avgCount;
            }
        }
    }

    if (p.pass == 3)
    {
        int2 offsetVoxID = voxelIDs[(pixel + int2(-1, -1)) * LIGHTING_DOWNSAMPLE];
        int2 offsetVoxID2 = voxelIDs[(pixel + int2(1, 1)) * LIGHTING_DOWNSAMPLE];
        int offsetFaceID = voxelFaceIDs[(pixel + int2(-1, -1)) * LIGHTING_DOWNSAMPLE];
        int offsetFaceID2 = voxelFaceIDs[(pixel + int2(1, 1)) * LIGHTING_DOWNSAMPLE];

        if (all(offsetVoxID != voxelID) || all(offsetVoxID2 != voxelID) || offsetFaceID != voxelFaceID || offsetFaceID2 != voxelFaceID)
        {
            float3 finalColor = indirectPerVoxPass2[pixel];
            var entry = get_hashtable_entry(p.voxel_hashmap_ptr, voxelID.x, voxelFaceID, p.state_ptr.frame);
            float lastTempFrames = entry.historyCount;
            float3 lastColor = entry.lighting;

            float blendFactor = 1.0 / (lastTempFrames);
            blendFactor = clamp(blendFactor, 0.01, 0.99);
            finalColor = lastColor * (1 - blendFactor) + finalColor * (blendFactor);

            voxel_hashtable_insert(p.voxel_hashmap_ptr, voxelID.x, voxelFaceID, finalColor, lastTempFrames + 1, p.state_ptr.frame);
        }
    }
}
