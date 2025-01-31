#include "atmosphere.slang"
#include "lygia/color/tonemap/unreal.hlsl"
#include "raycast.slang"
#include "voxel-hashmap.slang"

static const float AMBIENT_STRENGTH = 0.1;
static const float DEPTH_SCALE = 0.00002;
static const float FIREFLY_THRESHOLD = 4.0; // How much brighter than median before considering a firefly
static const float FIREFLY_REDUCTION = 0.5; // Strength of firefly reduction

bool isEffectivelyZero(float3 color)
{
    return length(color) < EPSILON;
}

float getLuminance(float3 color)
{
    return dot(color, float3(0.2126, 0.7152, 0.0722));
}

float3 removeFireflies(float3 color, uint2 pixel, RWTexture2D<float4> image)
{
    float centerLum = getLuminance(color);
    float lumSum = 0.0;
    float lumArray[8];
    int validSamples = 0;

    // Sample 3x3 neighborhood (excluding center)
    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            if (x == 0 && y == 0)
                continue;

            int2 samplePos = int2(pixel) + int2(x, y);

            // Check bounds
            if (samplePos.x >= 0 && samplePos.y >= 0 &&
                samplePos.x < p.frame_dim.x && samplePos.y < p.frame_dim.y)
            {
                float3 sampleColor = image[samplePos].rgb;
                float sampleLum = getLuminance(sampleColor);
                lumArray[validSamples++] = sampleLum;
                lumSum += sampleLum;
            }
        }
    }

    // Sort luminance values to find median (simple bubble sort since array is small)
    for (int i = 0; i < validSamples - 1; i++)
    {
        for (int j = 0; j < validSamples - i - 1; j++)
        {
            if (lumArray[j] > lumArray[j + 1])
            {
                float temp = lumArray[j];
                lumArray[j] = lumArray[j + 1];
                lumArray[j + 1] = temp;
            }
        }
    }

    float medianLum = lumArray[validSamples / 2];

    // If the pixel is significantly brighter than the median, reduce its intensity
    if (centerLum > medianLum * FIREFLY_THRESHOLD)
    {
        float reduction = lerp(1.0, medianLum / centerLum, FIREFLY_REDUCTION);
        return color * reduction;
    }

    return color;
}

float3 sampleSurroundingIndirect(RWTexture2D<float3> indirect, uint2 halfRes_pixel)
{
    float3 sum = 0.0;
    int validSamples = 0;

    // Sample 3x3 neighborhood
    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            // Skip center pixel
            if (x == 0 && y == 0)
                continue;

            int2 samplePos = int2(halfRes_pixel) + int2(x, y);

            // Check bounds
            if (samplePos.x >= 0 && samplePos.y >= 0 &&
                samplePos.x < p.frame_dim.x / 2 && samplePos.y < p.frame_dim.y / 2)
            {
                float3 sample = indirect[samplePos];
                if (!isEffectivelyZero(sample))
                {
                    sum += sample;
                    validSamples++;
                }
            }
        }
    }

    return validSamples > 0 ? sum / validSamples : float3(0, 0, 0);
}

[numthreads(8, 8, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    uint2 pixel = pixel_i.xy;
    if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
        return;

    RWTexture2D<float4> render_image = RWTexture2D<float4>::get(p.final_image);

    // Read from gbuffer textures
    RWTexture2D<float3> albedo = RWTexture2D<float3>::get(p.gbuffer.color);
    RWTexture2D<float3> normal = RWTexture2D<float3>::get(p.gbuffer.normal);
    RWTexture2D<float3> position = RWTexture2D<float3>::get(p.gbuffer.position);
    RWTexture2D<float3> indirect = RWTexture2D<float3>::get(p.gbuffer.indirect);
    RWTexture2D<float4> indirectHistory = RWTexture2D<float4>::get(p.gbuffer.indirectHistory);
    RWTexture2D<float2> motion = RWTexture2D<float2>::get(p.gbuffer.motion);
    RWTexture2D<float> depth = RWTexture2D<float>::get(p.gbuffer.depth);
    RWTexture2D<float> depthHalfRes = RWTexture2D<float>::get(p.gbuffer.depthHalfRes);
    RWTexture2D<int2> voxelIDs = RWTexture2D<int2>::get(p.gbuffer.voxelIDs);

    // Sample all buffers at current pixel
    float3 albedoColor = albedo[pixel];
    float3 normalVector = normal[pixel];
    float3 worldPos = position[pixel].xyz;
    float3 pixelLight = indirect[pixel / 2];
    float3 lastPixelLight = indirectHistory[pixel / 2].xyz;
    float2 motionVector = motion[pixel];
    float depthValue = depth[pixel];
    float depthValueHalfRes = depthHalfRes[pixel / 2];
    int2 voxelID = voxelIDs[pixel];

    uint slot = hashVoxel(voxelID.x, voxelID.y);

    uint32_t tries = 0;
    while (tries < 100)
    {
        if (p.voxel_hashmap_ptr.brickIndices[slot] == voxelID.x && p.voxel_hashmap_ptr.voxelIndices[slot] == voxelID.y)
        {
            break;
        }
        if (p.voxel_hashmap_ptr.brickIndices[slot] == EMPTY_KEY && p.voxel_hashmap_ptr.voxelIndices[slot] == EMPTY_KEY)
        {
            slot = EMPTY_KEY;
            break;
        }

        slot = (slot + 1) & (HASH_TABLE_SIZE - 1);
        tries++;
    }

    float3 voxelLight = p.voxel_hashmap_ptr.indirectLight[slot] / max(p.voxel_hashmap_ptr.hitCounts[slot], 1);
    int samples = p.voxel_hashmap_ptr.temporalFrames[slot];

    // Get ray and atmosphere setup
    float3 rayOrigin, rayDir;
    getRayFromPixel(pixel, p.frame_dim, p.state_ptr.camera, rayOrigin, rayDir);
    float3 sunDir = p.state_ptr.sunDir;
    AtmosphereParams atmosphere = initAtmosphere(rayDir, sunDir);

    // Handle background pixels (sky)
    if (depthValue > 1000.0f)
    {
        render_image[pixel] = tonemapUnreal(float4(getAtmosphereColor(atmosphere, p.state_ptr.time), 1.0));
        return;
    }

    // Get sun color from atmosphere for lighting
    float3 lightColor = getSunColor(atmosphere);
    float3 baseSkyColor = getBaseSkyColor(atmosphere);

    float3 finalColor = voxelLight * albedoColor.rgb;

    // Fog effect
    float fogFactor = saturate(depthValue * depthValue * DEPTH_SCALE);
    finalColor = lerp(finalColor, baseSkyColor, fogFactor);

    // Debug visualization modes
    if ((bool)p.state_ptr.settings.showNormals)
        finalColor = normalVector;
    if ((bool)p.state_ptr.settings.showHitPos)
        finalColor = worldPos / float(GRID_SIZE);
    if ((bool)p.state_ptr.settings.showDepth)
        finalColor = float3(depthValue / 100.0);
    if ((bool)p.state_ptr.settings.showDepthPrepass)
        finalColor = float3(depthValueHalfRes / 100.0);

    // finalColor = voxelLight;
    // finalColor = pixelLight;
    // finalColor = albedoColor.rgb;
    // finalColor = (abs(motionVector.xy) * p.frame_dim * 0.1).xyx;
    // finalColor = pow(finalColor, 5);
    // finalColor = float3((float2)pixel.xy / p.frame_dim.xy, 0);

    // Apply firefly removal after all other processing but before final output
    // finalColor = removeFireflies(finalColor, pixel, render_image);

    // post process
    finalColor = tonemapUnreal(finalColor);

    render_image[pixel] = float4(finalColor, 1.0);
}
