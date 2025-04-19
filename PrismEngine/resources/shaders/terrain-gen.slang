#include "common.slang"
#include "lygia/generative/random.hlsl"
#include "lygia/generative/snoise.hlsl"

// Terrain generation parameters
static const float noiseScale = 0.07f; 
static const uint blockSize = 12;
static const float baseHeight = 90.0f;

// Global scroll variable for terrain exploration
static const float2 terrainScroll = float2(0.0f, 0.0f);  // Change these values to explore different areas

// Improved FBM with better height variation
float fbm(float2 pos, int octaves, float lacunarity, float persistence, float baseFreq)
{
    float total = 0.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;
    float frequency = 1.0f;
    
    // Apply scroll offset
    pos += terrainScroll * 1000.0f;
    
    for(int i = 0; i < octaves; i++) {
        total += snoise(float3(pos.x, pos.y, i * 0.5f) * baseFreq * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    
    return total / maxValue;
}

// Improved terrain terrace function with natural variation
float terraceHeight(float height, float stepSize, float smoothing, float2 pos)
{
    // Add variation to step size based on position
    float positionVariation = snoise(float3(pos.x, pos.y, 456.7f) * 0.005f) * 0.3f + 1.0f;
    stepSize *= positionVariation;
    
    float steps = floor(height / stepSize);
    float remainder = frac(height / stepSize);
    
    // Varied smoothing for more natural transitions
    float actualSmoothing = smoothing * (0.5f + snoise(float3(pos.x, pos.y, 789.3f) * 0.01f) * 0.25f + 0.5f);
    actualSmoothing = clamp(actualSmoothing, 0.05f, 0.95f);
    
    // Smooth transition between steps
    float smoothedRemainder = actualSmoothing > 0.0f ? 
        smoothstep(0.0f, actualSmoothing, remainder) : remainder;
        
    return steps * stepSize + smoothedRemainder * stepSize;
}

// Cliff noise function - creates vertical drops
float cliffNoise(float2 pos, float scale)
{
    // Apply scroll
    pos += terrainScroll * 1000.0f;
    
    // Create noise patterns for cliff placement
    float noise = snoise(float3(pos.x, pos.y, 15.5f) * scale);
    
    // Sharpen the noise to create more defined cliff edges
    return pow(abs(noise), 0.5f) * sign(noise);
}

// Terrain type classification
float getTerrainType(float2 pos)
{
    // Apply scroll offset
    pos += terrainScroll * 1000.0f;
    
    // Large scale noise for terrain classification
    float terrainType = snoise(float3(pos.x, pos.y, 10.0f) * 0.0015f) * 0.5f + 0.5f;
    
    // Add medium-scale variation
    terrainType += snoise(float3(pos.x, pos.y, 20.0f) * 0.003f) * 0.25f;
    
    // Enhance contrast for more distinct terrain types
    terrainType = pow(terrainType, 0.9f);
    
    return saturate(terrainType);
}

// Convert grid coordinates to world position
float3 gridToWorldPos(int3 gridPos)
{
    return float3(gridPos) * WORLD_VOXEL_SIZE;
}

// Get world position aligned to blocks for consistent terrain
float3 getBlockAlignedPos(int3 gridPos)
{
    float3 worldPos = gridToWorldPos(gridPos);
    return float3(
        floor(worldPos.x / (WORLD_VOXEL_SIZE * blockSize)) * (WORLD_VOXEL_SIZE * blockSize),
        floor(worldPos.y / (WORLD_VOXEL_SIZE * blockSize)) * (WORLD_VOXEL_SIZE * blockSize),
        floor(worldPos.z / (WORLD_VOXEL_SIZE * blockSize)) * (WORLD_VOXEL_SIZE * blockSize));
}

// Rocky outcropping function
float rockyOutcropping(float2 pos, float baseHeight)
{
    // Apply scroll
    pos += terrainScroll * 1000.0f;
    
    // Use high-frequency noise for sharp features
    float sharpNoise1 = snoise(float3(pos.x, pos.y, 40.0f) * 0.03f);
    float sharpNoise2 = snoise(float3(pos.x, pos.y, 60.0f) * 0.05f);
    
    // Generate craggy shapes
    float cragginess = pow(abs(sharpNoise1 * sharpNoise2), 0.8f) * 15.0f;
    
    // Only apply to certain areas based on another noise pattern
    float mask = smoothstep(0.6f, 0.8f, snoise(float3(pos.x, pos.y, 30.0f) * 0.01f) * 0.5f + 0.5f);
    
    return cragginess * mask;
}

// Enhanced terrain height calculation with more natural features
float calculateTerrainHeight(float2 position)
{
    // Apply scroll offset
    position += terrainScroll * 1000.0f;
    
    // Apply non-uniform domain warping to break symmetry
    float2 warpAmount = float2(
        snoise(float3(position.y * 0.8f, position.x * 1.2f, 42.5f) * 0.003f),
        snoise(float3(position.x * 0.9f, position.y * 1.1f, 67.8f) * 0.003f)
    ) * 150.0f;
    float2 warpedPosition = position + warpAmount;
    
    // Base height
    float baseHeight = 90.0f;
    
    // Get terrain type with additional asymmetric warping
    float terrainType = getTerrainType(warpedPosition);
    
    // Add a second terrain type influence for more variation
    float secondaryType = snoise(float3(position.x * 1.3f, position.y * 0.7f, 123.4f) * 0.002f) * 0.5f + 0.5f;
    terrainType = lerp(terrainType, secondaryType, 0.3f); // Blend for more natural variation
    
    // Continental shape (very large scale)
    float continentShape = fbm(warpedPosition, 2, 2.0f, 0.6f, 0.0005f) * 0.5f + 0.5f;
    float continentHeight = continentShape * 30.0f;
    
    // Medium terrain features with directional bias for asymmetry
    float2 asymWarpedPos = position + float2(
        snoise(float3(position.x, position.y, 111.0f) * 0.005f),
        snoise(float3(position.y, position.x, 222.0f) * 0.005f)
    ) * 50.0f;
    float mediumFeatures = fbm(asymWarpedPos, 3, 2.0f, 0.6f, 0.003f) * 25.0f;
    mediumFeatures *= smoothstep(0.2f, 0.5f, terrainType); // Wider transition
    
    // Small-scale details with different warping
    float2 detailWarpedPos = position + float2(
        snoise(float3(position.y * 1.1f, position.x * 0.9f, 333.0f) * 0.01f),
        snoise(float3(position.x * 1.2f, position.y * 0.8f, 444.0f) * 0.01f)
    ) * 20.0f;
    float smallDetails = fbm(detailWarpedPos, 3, 2.2f, 0.5f, 0.01f) * 8.0f;
    smallDetails *= smoothstep(0.3f, 0.6f, terrainType); // Wider transition
    
    // Plains with gentle undulation - no terracing here
    float plains = 0.0f;
    if (terrainType < 0.45f) { // Slightly wider range
        float plainsNoise = fbm(position + float2(13.4f, 27.8f), 2, 1.5f, 0.5f, 0.002f);
        plains = plainsNoise * 10.0f * (1.0f - terrainType * 1.5f);
    }
    
    // Hills with selective terracing only in specific areas
    float hills = 0.0f;
    if (terrainType >= 0.3f && terrainType < 0.7f) {
        // Use different offsets for asymmetric hill placement
        float2 hillsOffset = float2(
            snoise(float3(position.x * 0.7f, position.y * 1.3f, 555.0f) * 0.004f),
            snoise(float3(position.y * 0.9f, position.x * 1.1f, 666.0f) * 0.004f)
        ) * 70.0f;
        
        float2 hillPos = position + hillsOffset;
        float hillFactor = smoothstep(0.3f, 0.4f, terrainType) * smoothstep(0.7f, 0.6f, terrainType);
        hills = fbm(hillPos, 3, 2.2f, 0.6f, 0.004f) * 35.0f * hillFactor;
        
        // Add more selective terracing to hills (only in ~10% of hill areas)
        float terraceFactor = snoise(float3(position.x * 1.4f, position.y * 0.6f, 25.0f) * 0.01f) * 0.5f + 0.5f;
        if (terraceFactor > 0.85f) { // Changed from 0.6f to 0.85f - much more selective
            hills = terraceHeight(hills, 6.0f, 0.5f, position); // More smoothing
        }
    }
    
    // Cliffs - more selective with asymmetric patterns
    float cliffFactor = 0.0f;
    if (terrainType > 0.45f) {
        float2 cliffOffset = float2(
            snoise(float3(position.x * 1.2f, position.y * 0.8f, 777.0f) * 0.006f),
            snoise(float3(position.y * 1.3f, position.x * 0.7f, 888.0f) * 0.006f)
        ) * 30.0f;
        
        float2 cliffPos = position + cliffOffset;
        float cliffPattern = cliffNoise(cliffPos, 0.007f);
        
        // More selective cliff bands with non-circular transitions
        float cliffSelector = snoise(float3(position.x * 0.8f, position.y * 1.2f, 101.0f) * 0.008f);
        if (abs(cliffPattern) > 0.75f && abs(cliffPattern) < 0.82f && cliffSelector > 0.1f) {
            cliffFactor = 15.0f * sign(cliffPattern);
        }
    }
    
    // Mountains with reduced terracing and more natural features
    float mountains = 0.0f;
    if (terrainType > 0.65f) {
        // Enhanced ridge-based mountain generation with stronger domain warping
        float2 mountainOffset = float2(
            snoise(float3(position.x * 0.9f, position.y * 1.1f, 999.0f) * 0.007f),
            snoise(float3(position.y * 1.2f, position.x * 0.8f, 111.1f) * 0.007f)
        ) * 150.0f; // Increased warping
        
        float2 warpedPos = position + mountainOffset;
        float mountainNoise = fbm(warpedPos, 4, 2.2f, 0.7f, 0.005f);
        
        // Create dramatic mountains
        mountains = pow(abs(mountainNoise), 1.1f) * 80.0f * smoothstep(0.65f, 0.8f, terrainType);
        
        // Greatly reduced terracing, only in very specific mountain areas (5% instead of 60%)
        float mountainTerraceMask = snoise(float3(position.x * 1.3f, position.y * 0.7f, 35.0f) * 0.008f) * 0.5f + 0.5f;
        float secondaryMask = snoise(float3(position.y * 0.8f, position.x * 1.2f, 45.0f) * 0.01f) * 0.5f + 0.5f;
        
        // Only apply terracing when both masks align, making it much more selective
        if (mountainTerraceMask > 0.8f && secondaryMask > 0.7f) {
            // More smoothing for less obvious steps
            float stepSize = lerp(8.0f, 12.0f, mountainTerraceMask);
            float smoothing = lerp(0.4f, 0.6f, mountainTerraceMask); // Increased smoothing
            mountains = terraceHeight(mountains, stepSize, smoothing, position);
        }
        
        // Add rocky outcroppings and cragginess to mountains
        mountains += rockyOutcropping(position, mountains);
        
        // Add occasional dramatic spires in mountain regions with asymmetric patterns
        float spireNoiseA = snoise(float3(position.x * 1.1f, position.y * 0.9f, 30.0f) * 0.01f);
        float spireNoiseB = snoise(float3(position.y * 1.2f, position.x * 0.8f, 60.0f) * 0.01f);
        float combinedSpireNoise = spireNoiseA * spireNoiseB;
        
        if (combinedSpireNoise > 0.6f && terrainType > 0.85f) {
            mountains += 35.0f * pow(combinedSpireNoise - 0.6f, 1.5f) / 0.4f;
        }
    }
    
    // Valleys with asymmetric patterns
    float valleyNoise = snoise(float3(position.x * 1.1f, position.y * 0.9f, 50.0f) * 0.004f);
    float secondValleyNoise = snoise(float3(position.y * 0.8f, position.x * 1.2f, 70.0f) * 0.005f);
    float valleys = 0.0f;
    
    // Only create valleys where both noise patterns overlap to avoid circular patterns
    if (valleyNoise > 0.65f && valleyNoise < 0.75f && secondValleyNoise > 0.4f && terrainType > 0.4f) {
        float valleyEdge = smoothstep(0.65f, 0.67f, valleyNoise) * smoothstep(0.75f, 0.73f, valleyNoise);
        valleys = -25.0f * valleyEdge * secondValleyNoise;
    }
    
    // Mesa formations - much more selective
    float mesaNoise = snoise(float3(position.x * 0.9f, position.y * 1.1f, 70.0f) * 0.003f) * 0.5f + 0.5f;
    float secondMesaNoise = snoise(float3(position.y * 1.2f, position.x * 0.8f, 90.0f) * 0.004f) * 0.5f + 0.5f;
    float mesa = 0.0f;
    
    // Only create mesas where both noise patterns align, making them less circular
    if (mesaNoise > 0.8f && secondMesaNoise > 0.7f && terrainType > 0.5f && terrainType < 0.7f) {
        float2 mesaOffset = float2(
            snoise(float3(position.y * 1.1f, position.x * 0.9f, 80.0f) * 0.01f),
            snoise(float3(position.x * 0.8f, position.y * 1.2f, 85.0f) * 0.01f)
        ) * 40.0f;
        
        float2 mesaPos = position + mesaOffset;
        float mesaShape = fbm(mesaPos, 2, 2.0f, 0.6f, 0.01f) * 20.0f; 
        // Create hard flat tops but maintain some variation
        mesa = terraceHeight(mesaShape, 20.0f, 0.15f, position); // Increased smoothing slightly
        mesa *= smoothstep(0.8f, 0.9f, mesaNoise) * smoothstep(0.7f, 0.8f, secondMesaNoise);
    }
    
    // Combine all features with proper weighting
    float finalHeight = baseHeight + continentHeight + plains + hills + mountains + 
                      mediumFeatures + smallDetails + valleys + cliffFactor + mesa;
    
    // Apply terracing only in very specific areas (< 3% of terrain)
    float globalTerraceMask = snoise(float3(position.x * 0.9f, position.y * 1.1f, 90.0f) * 0.002f) * 0.5f + 0.5f;
    float secondGlobalMask = snoise(float3(position.y * 1.2f, position.x * 0.8f, 95.0f) * 0.003f) * 0.5f + 0.5f;
    
    if (globalTerraceMask > 0.9f && secondGlobalMask > 0.8f && terrainType > 0.6f) {
        finalHeight = terraceHeight(finalHeight, 8.0f, 0.25f, position); // More smoothing
    }
    
    return finalHeight;
}

// Calculate if a position should be solid
bool calculateSolid(int3 gridPos)
{
    float3 worldBlockPos = getBlockAlignedPos(gridPos);
    float2 terrainPos = float2(worldBlockPos.x, worldBlockPos.z);
    
    // Get terrain height at this position
    float terrainHeight = calculateTerrainHeight(terrainPos);
    float terrainType = getTerrainType(terrainPos);
    
    // Generate caves
    float caveDensity = 0.0f;
    
    // Only generate caves below surface with a minimum depth
    if (worldBlockPos.y < terrainHeight - 12.0f && worldBlockPos.y > 30.0f)
    {
        // Apply scroll for cave exploration
        float3 cavePos = worldBlockPos;
        cavePos.xz += terrainScroll * 1000.0f;
        
        // Cave system noise
        float caveNoise = snoise(cavePos * 0.025f);
        float caveDetail = snoise(cavePos * 0.06f) * 0.3f;
        caveNoise += caveDetail;
        
        // More caves in mountains, fewer in plains
        float caveFrequency = 0.08f + terrainType * 0.1f;
        
        // More caves at deeper levels
        float depthFactor = smoothstep(terrainHeight - 15.0f, 45.0f, worldBlockPos.y);
        float caveProbability = caveFrequency + depthFactor * 0.15f;
        
        // Add occasional large cave chambers
        float chamberNoise = snoise(cavePos * 0.01f);
        if (chamberNoise > 0.7f) {
            caveProbability += 0.1f;
        }
        
        // Cave boundaries
        caveDensity = smoothstep(0.3f - caveProbability, 0.5f - caveProbability, caveNoise);
        
        // Add cave tunnels that connect chambers
        float tunnelNoise = snoise(cavePos * 0.015f);
        if (abs(tunnelNoise) < 0.05f) {
            caveDensity = max(caveDensity, 0.5f);
        }
    }
    
    // Surface is solid, underground is solid unless there's a cave
    return worldBlockPos.y < terrainHeight && caveDensity < 0.25f;
}

// Helper function to set voxel material safely
void setVoxelMaterial(uint brickIndex, uint8_t material)
{
    p.materialPtrBuffer.data[brickIndex] = material;
}

// Enhanced material selection with more natural patterns
uint selectMaterial(int3 brickPos, int3 voxelPos)
{
    float3 worldPos = gridToWorldPos(voxelPos);
    float2 terrainPos = float2(worldPos.x, worldPos.z);
    
    // Apply scroll
    terrainPos += terrainScroll * 1000.0f;
    
    // Add warping to break circular patterns in material placement
    float2 materialWarp = float2(
        snoise(float3(worldPos.z * 0.75f, worldPos.x * 1.25f, worldPos.y * 0.5f) * 0.02f),
        snoise(float3(worldPos.x * 0.8f, worldPos.y * 0.6f, worldPos.z * 1.4f) * 0.02f)
    ) * 15.0f;
    
    float2 warpedTerrainPos = terrainPos + materialWarp;
    
    // Get terrain height and type
    float terrainHeight = calculateTerrainHeight(terrainPos);
    float terrainType = getTerrainType(warpedTerrainPos); // Use warped position for type
    
    // Calculate depth below surface
    float depthBelowSurface = terrainHeight - worldPos.y;
    
    // Material variation noise - use multiple uncorrelated noise functions
    float baseNoise = snoise(float3(worldPos.x + terrainScroll.x * 1000.0f, 
                                    worldPos.y * 1.2f, 
                                    worldPos.z + terrainScroll.y * 1000.0f) * 0.03f);
    
    float detailNoise = snoise(float3(worldPos.z * 0.9f, 
                                      worldPos.x * 1.1f, 
                                      worldPos.y * 0.7f) * 0.05f);
    
    // Combine noise patterns in a non-linear way to avoid circular patterns
    float materialNoise = (baseNoise * 0.6f + detailNoise * 0.4f) * 0.5f + 0.5f;
    
    // Add directional bias to material variation
    float directionalBias = snoise(float3(worldPos.x - worldPos.z * 0.5f, 
                                          worldPos.y * 0.3f, 
                                          worldPos.z + worldPos.x * 0.5f) * 0.01f) * 0.25f;
    materialNoise = saturate(materialNoise + directionalBias);
    
    // Calculate slope for material selection - with jittered sample points for natural edges
    float2 sampleDist = float2(5.0f, 0.0f);
    float jitter = snoise(float3(terrainPos.x * 0.8f, terrainPos.y * 1.2f, 123.4f) * 0.1f) * 1.5f;
    
    float2 jitteredSampleX1 = sampleDist.xy * (1.0f + jitter * 0.2f);
    float2 jitteredSampleX2 = -sampleDist.xy * (1.0f - jitter * 0.2f);
    float2 jitteredSampleZ1 = sampleDist.yx * (1.0f + jitter * 0.2f);
    float2 jitteredSampleZ2 = -sampleDist.yx * (1.0f - jitter * 0.2f);
    
    float heightX1 = calculateTerrainHeight(terrainPos + jitteredSampleX1);
    float heightX2 = calculateTerrainHeight(terrainPos + jitteredSampleX2);
    float heightZ1 = calculateTerrainHeight(terrainPos + jitteredSampleZ1);
    float heightZ2 = calculateTerrainHeight(terrainPos + jitteredSampleZ2);
    
    float slopeX = abs(heightX1 - heightX2) / (length(jitteredSampleX1 - jitteredSampleX2));
    float slopeZ = abs(heightZ1 - heightZ2) / (length(jitteredSampleZ1 - jitteredSampleZ2));
    float slope = max(slopeX, slopeZ);
    
    // Add small random variation to slope detection
    slope *= (0.9f + snoise(float3(terrainPos.x, terrainPos.y, 567.8f) * 0.05f) * 0.2f);
    
    // Check if this is part of a cliff face - use gradual threshold for smoother transition
    float cliffFactor = smoothstep(0.75f, 0.85f, slope);
    
    // Check if this is on a terrace - more selective and varied
    float terraceInfluence = 0.0f;
    if (terrainType > 0.4f) {
        // Use non-uniform heights to avoid circular rings
        float baseTerraceHeight = 8.0f * (0.8f + materialNoise * 0.4f);
        float localHeight = frac(terrainHeight / baseTerraceHeight) * baseTerraceHeight;
        
        // Gradual influence rather than boolean
        float edgeWidth = 0.8f * (0.5f + materialNoise * 1.0f); // Variable width
        terraceInfluence = max(
            smoothstep(0.0f, edgeWidth, localHeight),
            smoothstep(baseTerraceHeight, baseTerraceHeight - edgeWidth, localHeight)
        );
        
        // Add directional variation to terrace detection
        terraceInfluence *= saturate((snoise(float3(terrainPos.y * 0.7f, terrainPos.x * 1.3f, 789.0f) * 0.02f) + 1.5f) * 0.5f);
    }
    
    // Surface layer with more varied transitions
    if (depthBelowSurface < 1.0f) 
    {
        // High mountains get snow/rock with gradual elevation-based transition
        if (terrainType > 0.7f && terrainHeight > 130.0f)
        {
            // Elevation-based gradual snow transition to avoid abrupt lines
            float snowFactor = smoothstep(130.0f, 140.0f, terrainHeight);
            
            // More rock on steeper slopes, smooth transition
            if (cliffFactor > 0.3f) {
                // Blend between snow and rock based on steepness
                return (materialNoise > lerp(0.8f, 0.2f, cliffFactor)) ? 6 : 2;
            }
            
            // Mix of snow and rock based on noise and elevation
            return (materialNoise > (0.7f - snowFactor * 0.4f)) ? 6 : 2;
        }
        // Cliffs get exposed rock regardless of elevation - smooth transition
        else if (cliffFactor > 0.1f)
        {
            // Blend cliff materials based on cliff factor
            if (materialNoise < cliffFactor * 0.8f) {
                return 2; // Rocky cliff faces
            }
            else {
                // Blend with surrounding terrain
                return (terrainHeight < 93.0f) ? 5 : 1;
            }
        }
        // Terraces get partial rock exposure based on influence
        else if (terraceInfluence > 0.3f && materialNoise > 0.5f)
        {
            // Blend between rock and normal terrain based on terrace influence
            if (materialNoise < terraceInfluence * 0.7f) {
                return 2; // Rocky terrace edges
            }
            else {
                // Blend with surrounding terrain
                return (terrainType > 0.6f) ? 2 : 1;
            }
        }
        // Hills and highlands with varied material distribution
        else if (terrainType > 0.3f && terrainType < 0.8f)
        {
            // Gradual increase in rock probability with slope
            float slopeRockChance = smoothstep(0.3f, 0.6f, slope) * 0.5f;
            
            // Rock appears more on slopes but with natural variation
            if (materialNoise > (0.9f - slopeRockChance)) {
                return 2;
            }
            return 1; // Mostly grass
        }
        // Near water level gets sandy with gradual transition
        else if (terrainHeight < 95.0f)
        {
            // Smooth transition from sand to grass as height increases
            float sandFactor = smoothstep(95.0f, 90.0f, terrainHeight);
            
            if (materialNoise < sandFactor * 0.8f) {
                return 5; // Sand/dirt
            }
            return 1; // Some grass patches even at lower elevations
        }
        // Plains with occasional variation
        else
        {
            // Very occasional rock outcroppings in plains
            if (materialNoise > 0.96f && snoise(float3(terrainPos.x * 1.5f, terrainPos.y * 0.9f, 876.5f) * 0.01f) > 0.7f) {
                return 2; // Rare rock in plains
            }
            return 1; // Mostly grass/soil
        }
    }
    // Shallow underground (topsoil/dirt layer) with better transitions
    else if (depthBelowSurface < 5.0f)
    {
        // Gradient between surface and deep materials
        float depthFactor = depthBelowSurface / 5.0f;
        
        // Rocky material near cliff faces even underground
        if (cliffFactor > 0.3f) {
            // Natural transition between materials
            if (materialNoise > (0.6f + depthFactor * 0.3f)) {
                return 5; // Some dirt even on cliff underground
            }
            return 2; // Mostly stone in cliff underground
        }
        
        // Gradual transition to stone with depth
        if (materialNoise > (0.2f + depthFactor * 0.7f)) {
            return 5; // Dirt/soil
        }
        return 2; // Stone increases with depth
    }
    // Deep underground (stone with occasional mineral deposits)
    else
    {
        // Use non-circular patterns for ore veins
        float oreNoisePrimary = snoise(float3(worldPos.x * 0.8f, worldPos.y * 1.2f, worldPos.z * 0.7f) * 0.05f);
        float oreNoiseSecondary = snoise(float3(worldPos.z * 1.1f, worldPos.x * 0.6f, worldPos.y * 0.9f) * 0.04f);
        
        // Create vein-like structures rather than circular deposits
        float veinPattern = (oreNoisePrimary * oreNoiseSecondary) * 0.5f + 0.5f;
        
        // Depth increases ore probability but with natural veining
        float depthFactor = min(depthBelowSurface / 200.0f, 0.5f);
        
        if (veinPattern > (0.85f - depthFactor) && materialNoise > 0.7f) {
            return 6; // Mineral/ore in natural vein patterns
        }
        else {
            return 2; // Stone
        }
    }
}

// Structure to hold brick and voxel indices
struct VoxelIndices
{
    int brickIndex;
    int voxelIndex;
    bool isValid;
};

// Get brick and voxel indices from world position
VoxelIndices getVoxelIndices(int3 worldPos)
{
    VoxelIndices result;

    // Calculate brick position
    int3 brickPos = worldPos / BRICK_SIZE;
    int3 localPos = worldPos % BRICK_SIZE;

    // Handle negative positions correctly
    if (any(localPos < 0))
    {
        brickPos -= 1;
        localPos = (localPos + BRICK_SIZE) % BRICK_SIZE;
    }

    // Check world boundaries
    int worldBlockSpan = GRID_SIZE * CHUNK_SIZE;
    if (any(brickPos < 0) || any(brickPos >= worldBlockSpan))
    {
        result.isValid = false;
        return result;
    }

    // Calculate indices
    int3 chunkPos = brickPos / CHUNK_SIZE;
    int3 localBrickPos = brickPos % CHUNK_SIZE;

    int chunkIndex = chunkIndex(chunkPos);
    int localBrickIndex = brickLocalIndex(localBrickPos);

    result.brickIndex = brickGlobalIndex(chunkIndex, localBrickIndex);
    result.voxelIndex = voxelLocalIndex(localPos);
    result.isValid = true;

    return result;
}

// Set a voxel at any world position
void setWorldVoxel(int3 worldPos, uint material)
{
    VoxelIndices indices = getVoxelIndices(worldPos);
    if (!indices.isValid)
        return;

    // Set material
    uint globalVoxelIndex = voxelGlobalIndex(indices.brickIndex, indices.voxelIndex);
    setVoxelMaterial(globalVoxelIndex, (uint8_t)material);  // Added explicit cast to fix warning

    // Update occupancy
    setBit(p.bricksBuffer.data[indices.brickIndex], indices.voxelIndex);
    setBit(p.bricksBuffer, indices.brickIndex / CHUNK_SIZE_CUBE, indices.brickIndex % CHUNK_SIZE_CUBE);
}

// Check if a world position contains a voxel
bool hasWorldVoxel(int3 worldPos)
{
    VoxelIndices indices = getVoxelIndices(worldPos);
    if (!indices.isValid)
        return false;

    BrickBitmask occupancy = p.bricksBuffer.data[indices.brickIndex];
    return getBit(occupancy, indices.voxelIndex);
}

[numthreads(4, 4, 4)]
void main(uint3 brickCoord: SV_DispatchThreadID, uint3 localBrickCoord: SV_GroupThreadID)
{
    int worldBlockSpan = GRID_SIZE * CHUNK_SIZE;

    if (brickCoord.x >= worldBlockSpan || brickCoord.y >= worldBlockSpan || brickCoord.z >= worldBlockSpan)
        return;

    int3 chunkCoord = int3(brickCoord / CHUNK_SIZE);
    int chunkIndex = chunkIndex(chunkCoord);

    int localBrickIndex = brickLocalIndex(localBrickCoord);
    int brickIndex = brickGlobalIndex(chunkIndex, localBrickIndex);

    // write to bricks as well
    p.bricksBuffer.data[0].data = {0, 0, 0, 0, 0, 0, 0, 0};
    p.bricksBuffer.data[1].data = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
    
    if (p.passNum == 1)
    {
        var absVoxel = brickCoord * BRICK_SIZE;
        bool isSolid = calculateSolid(absVoxel);
        p.brickPtrBuffer.data[brickIndex] = isSolid ? 1 : 0;
        setVoxelMaterial(brickIndex, selectMaterial(brickCoord, absVoxel));

        if (isSolid)
        {
            setBit(p.chunksBuffer, chunkIndex, localBrickIndex);
        }
        else
        {
            clearBit(p.chunksBuffer, chunkIndex, localBrickIndex);
        }
    }
}