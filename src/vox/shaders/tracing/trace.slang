#pragma once

#include "../common.slang"

TraceResult traceScene(Ray ray, float startDepth = 0, bool ignoreTransparent = false, bool coarse = false)
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

    // Return early if tMin is already beyond maxDist
    if (tMin > ray.maxDist)
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
        // Calculate current distance to decide if we should terminate
        // Estimate distance based on DDA state - using tMax values
        float minTMax = min(min(chunkDDA.tMax.x, chunkDDA.tMax.y), chunkDDA.tMax.z);
        float estimatedDist = minTMax - tMin;
        if (estimatedDist > ray.maxDist)
        {
            return result;
        }
        
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
            // Calculate estimated distance at brick level
            float brickEstDist = estimatedDist + 
                min(min(brickDDA.tMax.x, brickDDA.tMax.y), brickDDA.tMax.z) / float(CHUNK_SIZE);
            if (brickEstDist > ray.maxDist)
            {
                return result;
            }
            
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

                    if (coarse)
                    {
                        // Calculate position for coarse hit
                        float3 hitPos = chunkDDA.cell + brickDDA.cell / float(CHUNK_SIZE) + 
                                        voxelDDA.cell / float(BRICK_SIZE) / float(CHUNK_SIZE);
                        float hitDist = length(hitPos - startPos);
                        
                        if (hitDist > ray.maxDist)
                        {
                            return result;
                        }
                        
                        result.hit = true;
                        result.position = hitPos;
                        result.normal = normal;
                        result.distance = hitDist;
                        result.cell = chunkDDA.cell * CHUNK_SIZE + brickDDA.cell;
                        result.chunkIndex = chunkIdx;
                        result.brickIndex = brickIdx;
                        return result;   
                    }

                    while (isInBounds(voxelDDA.cell, int3(BRICK_SIZE)))
                    {
                        int voxelIdx = voxelLocalIndex(voxelDDA.cell);
                        if (getBit(brickMask, voxelIdx))
                        {
                            // Calculate potential hit position
                            float3 voxelUv = getDdaUvs(voxelDDA, voxelOrigin, ray.direction);
                            float3 hitPos = chunkDDA.cell + brickDDA.cell / float(CHUNK_SIZE) +
                                            voxelDDA.cell / float(BRICK_SIZE) / float(CHUNK_SIZE) +
                                            voxelUv / float(BRICK_SIZE) / float(CHUNK_SIZE);
                                            
                            float hitDist = length(hitPos - startPos);
                            if (hitDist > ray.maxDist)
                            {
                                return result;
                            }
                            
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
                            result.hit = true;
                            result.position = hitPos;
                            // result.normal = calculateVoxelNormal(brickIdxGlobal, voxelIdx, p.brick_occupancy_ptr);
                            result.normal = normal;
                            result.distance = hitDist;
                            result.cell = chunkDDA.cell * CHUNK_SIZE + brickDDA.cell * BRICK_SIZE + voxelDDA.cell;
                            result.chunkIndex = chunkIdx;
                            result.brickIndex = brickIdx;
                            result.voxelIndex = voxelIdx;
                            result.materialId = materialId;
                            result.voxelUVW = voxelUv;
                            result.brickUVW = brickUv;

                            return result;
                        }
                        normal = stepDDA(voxelDDA);
                        result.iterations++;
                    }
                }
            }
            normal = stepDDA(brickDDA);
            result.iterations++;
        }
        normal = stepDDA(chunkDDA);
        result.iterations++;
    }

    result.normal = 0;
    return result;
}