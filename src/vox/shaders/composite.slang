#include "atmosphere.slang"
#include "raycast.slang"

static const float AMBIENT_STRENGTH = 0.1;
static const float DEPTH_SCALE = 0.008;

[numthreads(8, 8, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    uint2 pixel = pixel_i.xy;
    if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
        return;

    RWTexture2D<float4> render_image = RWTexture2D<float4>::get(p.final_image);

    // Read from gbuffer textures
    RWTexture2D<float4> albedo = RWTexture2D<float4>::get(p.gbuffer.color);
    RWTexture2D<float4> normal = RWTexture2D<float4>::get(p.gbuffer.normal);
    RWTexture2D<float4> position = RWTexture2D<float4>::get(p.gbuffer.position);
    RWTexture2D<float4> indirect = RWTexture2D<float4>::get(p.gbuffer.indirect);
    RWTexture2D<float> depth = RWTexture2D<float>::get(p.gbuffer.depth);
    RWTexture2D<float> depthHalfRes = RWTexture2D<float>::get(p.gbuffer.depthHalfRes);
    RWTexture2D<int2> voxelIDs = RWTexture2D<int2>::get(p.gbuffer.voxelIDs);

    // Sample all buffers at current pixel
    float4 albedoColor = albedo[pixel];
    float3 normalVector = normalize((normal[pixel].xyz - 0.5) * 2.0);
    float3 worldPos = position[pixel].xyz;
    float3 indirectLight = indirect[pixel].xyz;
    float depthValue = depth[pixel];
    float depthValueHalfRes = depthHalfRes[pixel / 2];
    int2 voxelID = voxelIDs[pixel];

    // Get ray and atmosphere setup
    float3 rayOrigin, rayDir;
    getRayFromPixel(pixel, p.frame_dim, p.state_ptr.camera, rayOrigin, rayDir);
    float3 sunDir = p.state_ptr.sunDir;
    AtmosphereParams atmosphere = initAtmosphere(rayDir, sunDir);

    // Handle background pixels (sky)
    if (depthValue == 0.0f)
    {
        render_image[pixel] = float4(getAtmosphereColor(atmosphere, p.state_ptr.time), 1.0);
        return;
    }

    // Get sun color from atmosphere for lighting
    float3 lightColor = getSunColor(atmosphere);
    float3 baseSkyColor = getBaseSkyColor(atmosphere);

    float NdotL = max(dot(normalVector, sunDir), 0.0);
    float attenuation = 1.0 / (1.0 + DEPTH_SCALE * depthValue);

    // Combine lighting
    float3 directLight = lightColor * NdotL * attenuation;
    float3 ambientLight = lightColor * AMBIENT_STRENGTH;
    float3 finalColor = albedoColor.rgb * (directLight + ambientLight) +
                        indirectLight.rgb;

    // Fog effect
    float fogFactor = saturate(depthValue * DEPTH_SCALE);
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

    render_image[pixel] = float4(finalColor, albedoColor.a);
}
