#include "atmosphere.slang"
#include "lygia/color/tonemap/unreal.hlsl"
#include "tracing/hashmap.slang"
// Function to compute orthogonal axes to a given normal
void getOrthogonalAxes(float3 normal, out float3 tangent, out float3 bitangent)
{
    // Find a vector not collinear with normal
    float3 helper = abs(normal.x) > 0.99f ? float3(0, 1, 0) : float3(1, 0, 0);
    
    // Compute tangent and bitangent using cross products
    tangent = normalize(cross(normal, helper));
    bitangent = normalize(cross(normal, tangent));
}

// Function to get adjacent brick in a specific direction
int3 getAdjacentDirection(float3 dir, float3 normal, float3 tangent, float3 bitangent)
{
    // Project the direction onto the tangent-bitangent plane
    float dotT = dot(dir, tangent);
    float dotB = dot(dir, bitangent);
    
    // Determine the dominant component in the tangent-bitangent plane
    if (abs(dotT) > abs(dotB))
    {
        return dotT > 0 ? int3(tangent) : int3(-tangent);
    }
    else
    {
        return dotB > 0 ? int3(bitangent) : int3(-bitangent);
    }
}

// Get interpolated lighting from adjacent entries
float3 getInterpolatedLighting(uint voxelID, float3 normal, float2 uv, VoxelHashmapEntry currentEntry)
{
    // Compute orthogonal axes to the normal
    float3 tangent, bitangent;
    getOrthogonalAxes(normal, tangent, bitangent);
    
    // Get the UV coordinates in the tangent-bitangent coordinate system
    float2 planeUV = uv;
    
    // Get adjacent bricks in the tangent-bitangent plane
    // Define the directions to sample
    const int NUM_SAMPLES = 4;
    float3 directions[NUM_SAMPLES] = {
        tangent,       // +tangent
        -tangent,      // -tangent
        bitangent,     // +bitangent
        -bitangent     // -bitangent
    };
    
    // Get adjacent brick indices and face IDs
    uint adjacentIDs[NUM_SAMPLES];
    int faceIDs[NUM_SAMPLES];
    VoxelHashmapEntry adjacentEntries[NUM_SAMPLES];
    
    for (int i = 0; i < NUM_SAMPLES; i++)
    {
        int3 dir = int3(sign(directions[i]));
        adjacentIDs[i] = getAdjacentBrickIndex(voxelID, dir);
        faceIDs[i] = getFaceID(normal, adjacentIDs[i]);
        adjacentEntries[i] = get_hashtable_entry(p.voxel_hashmap_ptr, adjacentIDs[i], faceIDs[i], p.state_ptr.frame);
    }
    
    // Calculate bilinear interpolation weights
    float2 weights = float2(planeUV.x, planeUV.y);
    float4 bilinearWeights = float4(
        (1-weights.x) * (1-weights.y),  // Bottom-left: -tangent, -bitangent
        weights.x * (1-weights.y),      // Bottom-right: +tangent, -bitangent
        (1-weights.x) * weights.y,      // Top-left: -tangent, +bitangent
        weights.x * weights.y           // Top-right: +tangent, +bitangent
    );
    
    // Count valid entries and accumulate weighted lighting
    float3 interpolatedLighting = float3(0, 0, 0);
    float totalWeight = 0;
    
    // First pass: calculate total valid weight and accumulate lighting
    for (int i = 0; i < NUM_SAMPLES; i++)
    {
        if (adjacentEntries[i].historyCount > 0)
        {
            float weight = bilinearWeights[i];
            interpolatedLighting += adjacentEntries[i].lighting * weight;
            totalWeight += weight;
        }
    }
    
    // If no adjacent entries are valid, fall back to current entry
    if (totalWeight <= 0)
    {
        return currentEntry.historyCount > 0 ? currentEntry.lighting : float3(0, 0, 0);
    }
    
    // Normalize the accumulated lighting
    interpolatedLighting /= totalWeight;
    
    // Blend with current entry if it's valid
    if (currentEntry.historyCount > 0)
    {
        // Use a central weight bias
        float centralWeight = 0.6;
        interpolatedLighting = lerp(interpolatedLighting, currentEntry.lighting, centralWeight);
    }
    
    return interpolatedLighting;
}
[numthreads(8, 8, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    uint2 pixel = pixel_i.xy;
    if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
        return;

    // Retrieve our render targets and G-buffer textures.
    RWTexture2D<float4> render_image = RWTexture2D<float4>::get(p.final_image);
    RWTexture2D<float3> albedoTex = RWTexture2D<float3>::get(p.gbuffer.color);
    RWTexture2D<float3> indirectTex = RWTexture2D<float3>::get(p.gbuffer.indirectDenoised);
    RWTexture2D<float3> indirectPerVoxPassTex = RWTexture2D<float3>::get(p.gbuffer.indirectPerVoxelPass2);
    RWTexture2D<uint2> voxelIDTex = RWTexture2D<uint2>::get(p.gbuffer.voxelIDs);
    RWTexture2D<uint> voxelFaceIDTex = RWTexture2D<uint>::get(p.gbuffer.voxelFaceIDs);
    RWTexture2D<uint> materialIDTex = RWTexture2D<uint>::get(p.gbuffer.materialIDs);
    RWTexture2D<float2> voxelUVsTex = RWTexture2D<float2>::get(p.gbuffer.voxelUVs);
    RWTexture2D<float2> brickUVsTex = RWTexture2D<float2>::get(p.gbuffer.brickUVs);
    RWTexture2D<float> shadowTex = RWTexture2D<float>::get(p.gbuffer.shadow);
    RWTexture2D<float3> normalTex = RWTexture2D<float3>::get(p.gbuffer.normal);

    // Get the current voxel data.
    float3 indirect = indirectTex[pixel / 2];
    uint2 voxelID = voxelIDTex[pixel];
    uint voxelFaceID = voxelFaceIDTex[pixel];
    VoxelHashmapEntry entry = get_hashtable_entry(p.voxel_hashmap_ptr, voxelID.x, voxelFaceID, p.state_ptr.frame);

    // Get surface properties.
    float3 normal = normalTex[pixel];
    float2 brickUV = brickUVsTex[pixel];
    float3 albedo = albedoTex[pixel];

    // If there is no valid normal, we assume sky.
    if (normal.x == 0 && normal.y == 0 && normal.z == 0)
    {
        render_image[pixel] = float4(tonemapUnreal(albedo), 1.0);
        return;
    }

    float shadow = shadowTex[pixel];
    uint materialID = materialIDTex[pixel];
    materialID = clamp(materialID, 0, TOTAL_MATERIALS - 1);
    Material material = p.materials_ptr.materials[materialID];
    float ldot = max(dot(normal, p.state_ptr.sunDir), 0.0);

    // Compute a base color that includes albedo, sun contribution, and shadow.
    float3 color = albedo;
    color += shadow * ldot * getSunColor(initAtmosphere(normal, p.state_ptr.sunDir)) * albedo;
    color *= shadow / 2.0 + 1.0;

    // Determine the dominant axes for interpolation based on the normal.
    int3 axis1, axis2;
    if (abs(normal.x) > abs(normal.y) && abs(normal.x) > abs(normal.z)) {
        // X is dominant: use Y and Z.
        axis1 = int3(0, 1, 0);
        axis2 = int3(0, 0, 1);
    } else if (abs(normal.y) > abs(normal.z)) {
        // Y is dominant: use X and Z.
        axis1 = int3(1, 0, 0);
        axis2 = int3(0, 0, 1);
    } else {
        // Z is dominant: use X and Y.
        axis1 = int3(1, 0, 0);
        axis2 = int3(0, 1, 0);
    }

    // Use the brick UV coordinates (assumed to be in [0,1]) for bilinear interpolation.
    float u = brickUV.x;
    float v = brickUV.y;

    // Fetch the current voxel's lighting (center).
    float3 centerLighting = entry.historyCount > 0 ? entry.lighting : indirect;

    // Fetch the right neighbor along axis1.
    uint neighborRightID = getAdjacentBrickIndex(voxelID.x, axis1);
    int neighborRightFaceID = getFaceID(normal, neighborRightID);
    VoxelHashmapEntry neighborRight = get_hashtable_entry(p.voxel_hashmap_ptr, neighborRightID, neighborRightFaceID, p.state_ptr.frame);
    float3 rightLighting = neighborRight.historyCount > 0 ? neighborRight.lighting : centerLighting;

    // Fetch the up neighbor along axis2.
    uint neighborUpID = getAdjacentBrickIndex(voxelID.x, axis2);
    int neighborUpFaceID = getFaceID(normal, neighborUpID);
    VoxelHashmapEntry neighborUp = get_hashtable_entry(p.voxel_hashmap_ptr, neighborUpID, neighborUpFaceID, p.state_ptr.frame);
    float3 upLighting = neighborUp.historyCount > 0 ? neighborUp.lighting : centerLighting;

    // Fetch the diagonal neighbor (axis1 + axis2).
    uint neighborDiagID = getAdjacentBrickIndex(voxelID.x, axis1 + axis2);
    int neighborDiagFaceID = getFaceID(normal, neighborDiagID);
    VoxelHashmapEntry neighborDiag = get_hashtable_entry(p.voxel_hashmap_ptr, neighborDiagID, neighborDiagFaceID, p.state_ptr.frame);
    float3 diagLighting = neighborDiag.historyCount > 0 ? neighborDiag.lighting : centerLighting;

    // Perform standard bilinear interpolation:
    // First, interpolate horizontally between center and right, and up and diagonal.
    // Then, interpolate vertically between those two results.
    float3 lightingInterp = lerp(lerp(centerLighting, rightLighting, u),
                                  lerp(upLighting, diagLighting, u),
                                  v);

    // Final color: use the interpolated lighting and add any material emission.
    color *= entry.lighting;
    // color = indirect;
    // color += material.emission;

    // Apply tonemapping.
    color = tonemapUnreal(color);

    // Optionally, show normals.
    if (p.state_ptr.settings.showNormals)
    {
        color = normal * 0.5 + 0.5;
    }

    render_image[pixel] = float4(color, 1.0);
}
