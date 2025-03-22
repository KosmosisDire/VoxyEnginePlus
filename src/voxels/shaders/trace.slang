#include "tree64.slang"

[[vk::push_constant]]
const ComputePush push;

public float2 MartinR2(uint index) {
    return fract(index * float2(0.75487766624669276005, 0.56984029099805326591) + 0.5);
}

public float3 GetSphereDir(float2 sample) {
    float a = sample.x * 6.283185307179586;
    float y = sample.y * 2 - 1;

    float sy = sqrt(1.0 - y * y);
    return float3(sin(a) * sy, y, cos(a) * sy);
}

public void GetPrimaryRay(uint2 screenPos, out float3 rayPos, out float3 rayDir) {
    float2 uv = (screenPos + 0.5f) / float2(push.frame_dim);
    uv = uv * 2.0f - 1.0f;
    
    float4 farPlane = mul(push.invViewProjMat, float4(uv, 1.0f, 1.0f));
    rayDir = normalize(farPlane.xyz / farPlane.w);
    rayPos = push.cameraPos;
}

void getRayFromPixel(uint2 pixel, uint2 dimensions, out float3 rayPos, out float3 rayDir)
{
    rayPos = push.cameraPos;
    rayDir = float3(0.0);

    // Convert pixel to NDC space (-1 to 1)
    float2 uv = (float2(pixel) + 0.5f) / float2(dimensions);
    float2 ndc = uv * 2.0f - 1.0f;

    // Transform NDC position to world space
    float4 worldPos = mul(push.invViewProjMat, float4(ndc.x, ndc.y, 1.0f, 1.0f));
    worldPos.xyz /= worldPos.w;

    // Calculate ray direction
    rayDir = normalize(worldPos.xyz - push.cameraPos);
}