#include "common.slang"

struct TraceResult
{
    bool hit;
    float3 hitPos;
    float3 normal;
    float distance;
    float3 voxelUv;
    float3 brickUv;
    int3 voxelCell;
    int steps;
    int chunkIndex;
    int brickIndex;
    int voxelIndex;
};

#define continueDDA(state)   \
    normal = stepDDA(state); \
    result.steps++;          \
    continue;

TraceResult traceVoxelRay(float3 origin, float3 direction, float startDepth = 0, int maxSteps = MAX_STEPS)
{
    float3 trueStart = origin;
    origin += direction * startDepth;

    float3 invDir = inverseDirection(direction);

    TraceResult result = (TraceResult)0;

    // intersect with bounding box
    float tMin, tMax;
    float3 normal;
    bool boxHit = intersectBoxNormal(origin, invDir, float3(0.0), float3(GRID_SIZE), tMin, tMax, normal);
    if (!boxHit)
    {
        return result;
    }

    // adjust origin to be inside the box
    if (tMin > EPSILON)
    {
        origin += direction * (tMin + EPSILON);
    }

    DDAState chunkDDA = initDDA(origin, direction, invDir);
    while (result.steps < maxSteps && isInBounds(chunkDDA.cell, int3(GRID_SIZE)))
    {
        int chunkIdx = chunkIndex(chunkDDA.cell);
        daxa_u32vec2 chunkMask = p.chunk_occupancy_ptr.occupancy[chunkIdx];

        if (all(chunkMask == 0))
        {
            continueDDA(chunkDDA);
        }

        float3 chunkUv = getDdaUvs(chunkDDA, origin, direction);

        float3 brickOrigin = clamp(chunkUv * CHUNK_SIZE, float3(EPSILON), float3(CHUNK_SIZE - EPSILON));
        DDAState brickDDA = initDDA(brickOrigin, direction, invDir);
        while (result.steps < maxSteps && isInBounds(brickDDA.cell, int3(CHUNK_SIZE)))
        {
            int brickIdx = brickLocalIndex(brickDDA.cell);
            if (getBit(chunkMask, brickIdx))
            {
                int brickIdxGlobal = brickGlobalIndex(chunkIdx, brickIdx);
                BrickBitmask brickMask = p.brick_occupancy_ptr.occupancy[brickIdxGlobal];

                if (brickBitmaskEmpty(brickMask))
                {
                    continueDDA(brickDDA);
                }

                float3 brickUv = getDdaUvs(brickDDA, brickOrigin, direction);

                float3 voxelOrigin = clamp(brickUv * BRICK_SIZE, float3(EPSILON), float3(BRICK_SIZE - EPSILON));
                DDAState voxelDDA = initDDA(voxelOrigin, direction, invDir);
                while (result.steps < maxSteps && isInBounds(voxelDDA.cell, int3(BRICK_SIZE)))
                {
                    int voxelIdx = voxelLocalIndex(voxelDDA.cell);

                    if (getBit(brickMask, voxelIdx))
                    {
                        float3 voxelUv = getDdaUvs(voxelDDA, voxelOrigin, direction);
                        float3 hitPos = chunkDDA.cell + brickDDA.cell / float(CHUNK_SIZE) + voxelDDA.cell / float(BRICK_SIZE) / float(CHUNK_SIZE) + voxelUv / float(BRICK_SIZE) / float(CHUNK_SIZE);

                        result.hit = true;
                        result.hitPos = hitPos;
                        result.normal = normal;
                        result.distance = length(hitPos - trueStart);
                        result.voxelUv = voxelUv;
                        result.brickUv = brickUv;
                        result.voxelCell = voxelDDA.cell;
                        result.chunkIndex = chunkIdx;
                        result.brickIndex = brickIdx;
                        result.voxelIndex = voxelIdx;

                        return result;
                    }

                    continueDDA(voxelDDA);
                }
            }

            continueDDA(brickDDA);
        }

        continueDDA(chunkDDA);
    }

    return result;
}

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
