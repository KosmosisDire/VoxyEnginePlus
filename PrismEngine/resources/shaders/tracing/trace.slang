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
        ray.origin += ray.direction * (tMin + 0.01);
    }


    // Begin DDA traversal
    DDAState chunkDDA = initDDA(ray.origin, ray.direction, invDir);
    
    while (result.iterations < ray.maxIterations && isInBounds(chunkDDA.cell, int3(GRID_SIZE)))
    {
        int chunkIdx = chunkIndex(chunkDDA.cell);
        daxa_u32vec2 chunkMask = p.chunksBuffer.data[chunkIdx];

        if (all(chunkMask == 0))
        {
            normal = stepDDA(chunkDDA);
            continue;
        }

        // Brick traversal
        float3 chunkUv = getDdaUvs(chunkDDA, ray.origin, ray.direction);
        float3 brickOrigin = clamp(chunkUv * CHUNK_SIZE, float3(EPSILON), float3(CHUNK_SIZE - EPSILON));
        DDAState brickDDA = initDDA(brickOrigin, ray.direction, invDir);

        while (result.iterations < ray.maxIterations && isInBounds(brickDDA.cell, int3(CHUNK_SIZE)))
        {
            int brickIdx = brickLocalIndex(brickDDA.cell);
            if (getBit(chunkMask, brickIdx))
            {
                int brickIdxGlobal = brickGlobalIndex(chunkIdx, brickIdx);
                BrickBitmask brickMask = p.bricksBuffer.data[(int)p.brickPtrBuffer.data[brickIdxGlobal]];

                if (!brickBitmaskEmpty(brickMask))
                {
                    // Voxel traversal
                    float3 brickUv = getDdaUvs(brickDDA, brickOrigin, ray.direction);
                    float3 voxelOrigin = clamp(brickUv * BRICK_SIZE, float3(EPSILON), float3(BRICK_SIZE - EPSILON));
                    DDAState voxelDDA = initDDA(voxelOrigin, ray.direction, invDir);

                    if (coarse || result.iterations > ray.maxIterations - 1)
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

                    while (result.iterations < ray.maxIterations && isInBounds(voxelDDA.cell, int3(BRICK_SIZE)))
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
                            uint materialId = getMaterialId(brickIdxGlobal);

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

bool traceBrickForAO(float3 origin, float3 direction, float maxDist, out float hitDistance)
{
    float3 invDir = 1.0 / max(abs(direction), float3(EPSILON));
    invDir *= sign(direction);
    
    // Box intersection test
    float tMin, tMax;
    float3 normal;
    if (!intersectBoxNormal(origin, invDir, float3(0), float3(GRID_SIZE), tMin, tMax, normal))
    {
        hitDistance = maxDist;
        return false;
    }
    
    if (tMin > maxDist)
    {
        hitDistance = maxDist;
        return false;
    }
    
    float3 rayStart = origin;
    if (tMin > EPSILON)
    {
        origin += direction * (tMin + 0.01);
    }
    
    // Chunk traversal
    DDAState chunkDDA = initDDA(origin, direction, invDir);
    int iterations = 0;
    const int maxIterations = 60; // Enough for AO range
    
    while (iterations < maxIterations && isInBounds(chunkDDA.cell, int3(GRID_SIZE)))
    {
        int chunkIdx = chunkIndex(chunkDDA.cell);
        daxa_u32vec2 chunkMask = p.chunksBuffer.data[chunkIdx];
        
        if (all(chunkMask == 0))
        {
            normal = stepDDA(chunkDDA);
            iterations++;
            continue;
        }
        
        // Brick traversal - must match original logic exactly
        float3 chunkUv = getDdaUvs(chunkDDA, origin, direction);
        float3 brickOrigin = clamp(chunkUv * CHUNK_SIZE, float3(EPSILON), float3(CHUNK_SIZE - EPSILON));
        DDAState brickDDA = initDDA(brickOrigin, direction, invDir);
        
        while (iterations < maxIterations && isInBounds(brickDDA.cell, int3(CHUNK_SIZE)))
        {
            int brickIdx = brickLocalIndex(brickDDA.cell);
            if (getBit(chunkMask, brickIdx))
            {
                int brickIdxGlobal = brickGlobalIndex(chunkIdx, brickIdx);
                BrickBitmask brickMask = p.bricksBuffer.data[(int)p.brickPtrBuffer.data[brickIdxGlobal]];
                
                if (!brickBitmaskEmpty(brickMask))
                {
                    // Calculate exact hit position like in original
                    float3 brickUv = getDdaUvs(brickDDA, brickOrigin, direction);
                    float3 hitPos = chunkDDA.cell + brickDDA.cell / float(CHUNK_SIZE) + 
                                   brickUv / float(CHUNK_SIZE);
                    hitDistance = length(hitPos - rayStart);
                    
                    if (hitDistance <= maxDist)
                    {
                        // Check if emissive and skip if so
                        uint materialId = getMaterialId(brickIdxGlobal);
                        Material mat = getMaterial(materialId);
                        if (any(mat.emission != 0.0f))
                        {
                            normal = stepDDA(brickDDA);
                            iterations++;
                            continue;
                        }
                        
                        return true;
                    }
                }
            }
            normal = stepDDA(brickDDA);
            iterations++;
        }
        normal = stepDDA(chunkDDA);
        iterations++;
    }
    
    hitDistance = maxDist;
    return false;
}

float3 RandomHemisphereUnitVector(uint seed1, uint seed2)
{
    // Simple hash for speed
    seed1 = seed1 * 747796405u + 2891336453u;
    seed2 = seed2 * 1664525u + 1013904223u;
    
    // Convert to float in [0,1) range
    float r1 = float(seed1) / 4294967296.0;
    float r2 = float(seed2) / 4294967296.0;
    
    // Simpler hemisphere generation
    float cosTheta = r1;
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    float phi = r2 * 6.28318530718;
    
    return float3(
        sinTheta * cos(phi),
        sinTheta * sin(phi),
        cosTheta
    );
}