#include "../atmosphere.slang"
#include "common.slang"
#include "hashmap.slang"

// Schlick's approximation for Fresnel reflection
float fresnelSchlick(float cosTheta, float F0)
{
    return pow(F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0), 4.0);
}

float random(float seed)
{
    return frac(sin(seed) * 43758.5453);
}

PixelResult shadePixel(SceneHitInfo hits, float3 rayDir, uint2 pixel, float3 blueNoise)
{
    PixelResult result = (PixelResult)0;

    if (!hits.solid.hit)
    {
        result.color = getAtmosphereColor(initAtmosphere(rayDir, p.state_ptr.sunDir), p.state_ptr.time);
        return result;
    }

    // Setup initial result values
    result.normal = hits.solid.normal;
    result.position = hits.solid.position;
    result.depth = hits.solid.distance;
    result.voxelId = int2(brickGlobalIndex(hits.solid.chunkIndex, hits.solid.brickIndex), hits.solid.voxelIndex);

    Material material = getMaterial(hits.solid.materialId);
    float3 color = material.albedo;

    // per voxel noise
    float voxelNoise = random(getFaceID(hits.solid.normal, hits.solid.brickIndex + hits.solid.voxelIndex * BRICK_SIZE_CUBE));
    color = clamp(color - voxelNoise * 0.06, 0, 1);

    result.color = color;
    return result;
}
