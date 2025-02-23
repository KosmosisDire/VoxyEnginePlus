#include "common.slang"

Ray getRayFromPixel(uint2 pixel, uint2 dimensions, CameraData camera)
{
    Ray ray;
    ray.origin = camera.position;
    ray.maxDist = camera.far;

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

TraceResult traceScene(Ray ray, float startDepth = 0, bool ignoreTransparent = false)
{
    float3 startPos = ray.origin;
    ray.origin += ray.direction * startDepth;

    float3 invDir = 1.0 / max(abs(ray.direction), float3(EPSILON));
    invDir *= sign(ray.direction);

    TraceResult result = (TraceResult)0;
    result.hit = false;
    result.distance = ray.maxDist;
    result.position = ray.origin + ray.direction * ray.maxDist;

    int transparentVoxelDepth = 0;

    // First box intersection test
    float tMin, tMax;
    float3 normal;
    if (!intersectBoxNormal(ray.origin, invDir, float3(0), float3(GRID_SIZE), tMin, tMax, normal))
    {
        return result;
    }

    // Adjust start position if needed
    if (tMin > EPSILON)
    {
        ray.origin += ray.direction * (tMin + EPSILON);
        normal = -sign(ray.direction);
    }

    // Begin DDA traversal
    DDAState chunkDDA = initDDA(ray.origin, ray.direction, invDir);
    while (isInBounds(chunkDDA.cell, int3(GRID_SIZE)))
    {
        int chunkIdx = chunkIndex(chunkDDA.cell);
        daxa_u32vec2 chunkMask = p.chunk_occupancy_ptr.occupancy[chunkIdx];

        if (all(chunkMask == 0))
        {
            normal = stepDDA(chunkDDA);
            continue;
        }

        // Brick traversal
        float3 chunkUv = getDdaUvs(chunkDDA, ray.origin, ray.direction);
        float3 brickOrigin = clamp(chunkUv * CHUNK_SIZE, float3(EPSILON), float3(CHUNK_SIZE - EPSILON));
        DDAState brickDDA = initDDA(brickOrigin, ray.direction, invDir);

        while (isInBounds(brickDDA.cell, int3(CHUNK_SIZE)))
        {
            int brickIdx = brickLocalIndex(brickDDA.cell);
            if (getBit(chunkMask, brickIdx))
            {
                int brickIdxGlobal = brickGlobalIndex(chunkIdx, brickIdx);
                BrickBitmask brickMask = p.brick_occupancy_ptr.occupancy[brickIdxGlobal];

                if (!brickBitmaskEmpty(brickMask))
                {
                    // Voxel traversal
                    float3 brickUv = getDdaUvs(brickDDA, brickOrigin, ray.direction);
                    float3 voxelOrigin = clamp(brickUv * BRICK_SIZE, float3(EPSILON), float3(BRICK_SIZE - EPSILON));
                    DDAState voxelDDA = initDDA(voxelOrigin, ray.direction, invDir);

                    while (isInBounds(voxelDDA.cell, int3(BRICK_SIZE)))
                    {
                        int voxelIdx = voxelLocalIndex(voxelDDA.cell);
                        if (getBit(brickMask, voxelIdx))
                        {
                            // Get material to check transparency
                            uint materialId = getMaterialId(int2(brickIdxGlobal, voxelIdx));

                            // Skip transparent materials if requested
                            if (ignoreTransparent)
                            {
                                Material mat = getMaterial(materialId);
                                if (mat.transparency > EPSILON)
                                {
                                    if (transparentVoxelDepth > 30)
                                    {
                                        break;
                                    }

                                    transparentVoxelDepth++;
                                    normal = stepDDA(voxelDDA);
                                    continue;
                                }
                            }

                            // Hit found
                            float3 voxelUv = getDdaUvs(voxelDDA, voxelOrigin, ray.direction);
                            float3 hitPos = chunkDDA.cell + brickDDA.cell / float(CHUNK_SIZE) +
                                            voxelDDA.cell / float(BRICK_SIZE) / float(CHUNK_SIZE) +
                                            voxelUv / float(BRICK_SIZE) / float(CHUNK_SIZE);

                            result.hit = true;
                            result.position = hitPos;
                            result.normal = normal;
                            result.distance = length(hitPos - startPos);
                            result.cell = chunkDDA.cell * CHUNK_SIZE + brickDDA.cell * BRICK_SIZE + voxelDDA.cell;
                            result.chunkIndex = chunkIdx;
                            result.brickIndex = brickIdx;
                            result.voxelIndex = voxelIdx;
                            result.materialId = materialId;

                            return result;
                        }
                        normal = stepDDA(voxelDDA);
                    }
                }
            }
            normal = stepDDA(brickDDA);
        }
        normal = stepDDA(chunkDDA);
    }

    return result;
}

SceneHitInfo traceAllRays(Ray primaryRay)
{
    SceneHitInfo hits = (SceneHitInfo)0;

    // Trace primary ray
    hits.solid = traceScene(primaryRay, 0, false);

    if (!hits.solid.hit)
    {
        return hits;
    }

    Material primaryMat = getMaterial(hits.solid.materialId);

    // Trace transmission ray if material is transparent
    if (primaryMat.transparency > EPSILON)
    {
        hits.transparent = hits.solid;
        hits.solid = traceScene(primaryRay, hits.solid.distance, true);

        if (primaryMat.reflectivity > EPSILON)
        {
            Ray reflRay;
            reflRay.origin = hits.transparent.position + hits.transparent.normal * EPSILON;
            reflRay.direction = reflect(primaryRay.direction, hits.transparent.normal);
            reflRay.maxDist = MAX_TRACE_DISTANCE;

            hits.reflection = traceScene(reflRay, 0, false);
        }
    }

    // Trace reflection ray if material is reflective
    else if (primaryMat.reflectivity > EPSILON)
    {
        Ray reflRay;
        reflRay.origin = hits.solid.position + hits.solid.normal * EPSILON;
        reflRay.direction = reflect(primaryRay.direction, hits.solid.normal);
        reflRay.maxDist = MAX_TRACE_DISTANCE;

        hits.reflection = traceScene(reflRay, 0, false);
    }

    return hits;
}
