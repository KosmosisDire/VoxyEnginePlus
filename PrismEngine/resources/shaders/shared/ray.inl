#pragma once
#include "daxa/daxa.inl"
#include "const.inl"

// Core ray structure
struct Ray
{
    daxa_f32vec3 origin;
    daxa_f32vec3 direction;
    daxa_f32 maxDist;
    daxa_u32 maxIterations = 1024;

#ifndef __cplusplus
    // Add these constants for jittering
    static const int HALTON_SEQUENCE_LENGTH = 16; // Number of jitter patterns to cycle through
    static const float JITTER_SCALE = 0.6f; // Size of jitter (0.5 = half a pixel)

    // Halton sequence for jittering (base 2 and 3)
    // These provide a well-distributed sampling pattern
    static const float2 HALTON_SAMPLES[HALTON_SEQUENCE_LENGTH] =
    {
        float2(0.0f, 0.0f),
        float2(0.5f, 0.33333f),
        float2(0.25f, 0.66666f),
        float2(0.75f, 0.11111f),
        float2(0.125f, 0.44444f),
        float2(0.625f, 0.77777f),
        float2(0.375f, 0.22222f),
        float2(0.875f, 0.55555f),
        float2(0.0625f, 0.88888f),
        float2(0.5625f, 0.03703f),
        float2(0.3125f, 0.37037f),
        float2(0.8125f, 0.70370f),
        float2(0.1875f, 0.14814f),
        float2(0.6875f, 0.48148f),
        float2(0.4375f, 0.81481f),
        float2(0.9375f, 0.25925f)
    };

    // Get jitter offset for current frame
    static float2 getJitterOffset(uint frameIndex)
    {
        // Cycle through the halton sequence based on frame number
        uint sampleIndex = frameIndex % HALTON_SEQUENCE_LENGTH;
        // Convert from [0,1] to [-0.5,0.5] range and scale by jitter amount
        return (HALTON_SAMPLES[sampleIndex] - 0.5f) * JITTER_SCALE;
    }

    static Ray FromPixelJitter(uint2 pixel, uint2 dimensions, CameraData camera)
    {
        Ray ray;
        ray.origin = camera.position;
        ray.maxDist = camera.far * 50;

        // Get jitter offset for current frame
        float2 jitter = getJitterOffset(p.stateBuffer.frame);

        // Convert pixel to NDC space (-1 to 1) with jitter
        float2 uv = (float2(pixel) + 0.5f + jitter) / float2(dimensions);
        float2 ndc = uv * 2.0f - 1.0f;

        // Transform NDC position to world space
        float4 worldPos = mul(camera.invViewProj, float4(ndc.x, ndc.y, 1.0f, 1.0f));
        worldPos.xyz /= worldPos.w;

        // Calculate ray direction
        ray.direction = normalize(worldPos.xyz - camera.position);
        return ray;
    }

    static Ray FromPixel(uint2 pixel, uint2 dimensions, CameraData camera)
    {
        Ray ray;
        ray.origin = camera.position;
        ray.maxDist = camera.far * 50;

        // Convert pixel to NDC space (-1 to 1)
        float2 uv = (float2(pixel) + 0.5f) / float2(dimensions);
        float2 ndc = uv * 2.0f - 1.0f;

        // Transform NDC position to world space
        float4 worldPos = mul(camera.invViewProj, float4(ndc.x, ndc.y, 1.0f, 1.0f));
        worldPos.xyz /= worldPos.w;

        // Calculate ray direction
        ray.direction = normalize(worldPos.xyz - camera.position);
        return ray;
    }

#endif

};