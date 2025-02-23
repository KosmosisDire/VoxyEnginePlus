#include "../atmosphere.slang"
#include "../voxel-hashmap.slang"
#include "common.slang"

uint getFaceID(float3 normal, int voxelIndex)
{
    uint faceID = (normal.x != 0 ? 1 : 0) +
                  (normal.y != 0 ? 2 : 0) +
                  (normal.z != 0 ? 4 : 0);
    faceID = faceID * 216 + voxelIndex;
    return faceID;
}

VoxelHashmapEntry getHashmapEntry(TraceResult trace)
{
    if (!trace.hit)
        return (VoxelHashmapEntry)0;

    int globalBrickIndex = brickGlobalIndex(trace.chunkIndex, trace.brickIndex);
    return get_hashtable_entry(p.voxel_hashmap_ptr, globalBrickIndex, getFaceID(trace.normal, trace.voxelIndex), p.state_ptr.frame);
}

void setHashmapEntry(TraceResult trace, float3 lighting, int historyCount)
{
    if (!trace.hit)
        return;

    int globalBrickIndex = brickGlobalIndex(trace.chunkIndex, trace.brickIndex);
    uint faceID = getFaceID(trace.normal, trace.voxelIndex);
    voxel_hashtable_insert(p.voxel_hashmap_ptr, globalBrickIndex, faceID, lighting, historyCount, p.state_ptr.frame);
}

float3 getVoxelLighting(TraceResult trace)
{
    if (!trace.hit)
        return float3(0, 0, 0);

    var entry = getHashmapEntry(trace);
    return entry.lighting;
}

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

    // Setup initial result values
    result.hitAnything = hits.solid.hit || hits.transparent.hit || hits.reflection.hit;
    result.normal = hits.solid.normal;
    result.position = hits.solid.position;
    result.depth = hits.solid.distance;

    if (!result.hitAnything)
    {
        result.color = getBaseSkyColor(rayDir, p.state_ptr.sunDir);
        return result;
    }

    // Get primary surface properties
    result.voxelId = int2(brickGlobalIndex(hits.solid.chunkIndex, hits.solid.brickIndex), hits.solid.voxelIndex);
    Material primaryMat = getMaterial(hits.solid.materialId);
    float3 primaryColor = primaryMat.albedo + primaryMat.emission;

    // Get transparent material properties
    Material transMat = getMaterial(hits.transparent.materialId);
    float transparentDepth = hits.solid.distance - hits.transparent.distance;

    float3 finalColor = primaryColor;

    // Calculate transparency and reflection effects if present
    if (transMat.transparency > EPSILON)
    {
        float depth = clamp(1.0 - length(transparentDepth) / 2.0, 0.0, transMat.transparency);
        finalColor = lerp(transMat.albedo * 0.05, primaryColor, depth) * transMat.albedo;

        if (blueNoise.x < transMat.transparency)
        {
            result.voxelId = int2(brickGlobalIndex(hits.transparent.chunkIndex, hits.transparent.brickIndex), hits.transparent.voxelIndex);
            result.position = hits.transparent.position;
            result.normal = hits.transparent.normal;
        }

        // Calculate view angle for Fresnel
        float cosTheta = abs(dot(normalize(hits.transparent.normal), normalize(-rayDir)));

        float F0 = transMat.reflectivity;
        float fresnelFactor = clamp(fresnelSchlick(cosTheta, F0), 0, 1);

        float3 reflectedColor = hits.reflection.hit ? getMaterial(hits.reflection.materialId).albedo : getBaseSkyColor(reflect(rayDir, hits.transparent.normal), p.state_ptr.sunDir);
        float reflectionStrength = clamp(F0 * (1 - depth) * fresnelFactor * 100, 0, 1);
        finalColor = lerp(finalColor, reflectedColor, reflectionStrength);

        if (blueNoise.y < reflectionStrength)
        {
            result.voxelId = int2(brickGlobalIndex(hits.reflection.chunkIndex, hits.reflection.brickIndex), hits.reflection.voxelIndex);
            result.position = hits.reflection.position;
            result.normal = hits.reflection.normal;
        }
    }
    else if (primaryMat.reflectivity > EPSILON)
    {
        float3 reflectedColor = hits.reflection.hit ? getMaterial(hits.reflection.materialId).albedo : getBaseSkyColor(reflect(rayDir, hits.solid.normal), p.state_ptr.sunDir);
        finalColor = lerp(finalColor, reflectedColor, primaryMat.reflectivity);

        if (blueNoise.y < primaryMat.reflectivity)
        {
            result.voxelId = int2(brickGlobalIndex(hits.reflection.chunkIndex, hits.reflection.brickIndex), hits.reflection.voxelIndex);
            result.position = hits.reflection.position;
            result.normal = hits.reflection.normal;
        }
    }

    result.color = finalColor;
    return result;
}
