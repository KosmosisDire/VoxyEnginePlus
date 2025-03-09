#include "shared.inl"

[[vk::push_constant]]
const ComputePush p;

// ===== CONFIGURATION =====
// Number of procedural lights
static const uint LIGHT_COUNT = 100;  // Changed from 0 to 8 for testing

// Light propagation parameters
static const float EMISSION_STRENGTH = 2.0;
static const float SMOOTH_PROP_FACTOR = 0.999;    // Slightly lower for better responsiveness
static const float SMOOTH_BLEND_WEIGHT = 0.995;    // Less averaging to allow faster propagation

// Occlusion parameters
static const uint OCCLUDER_PATTERN = 2;       // 0=none, 1=walls, 2=maze, 3=random blocks
static const float OCCLUDER_DENSITY = 0.08;   // For random pattern

// Occluder color (default/ambient color)
static const float3 OCCLUDER_COLOR = float3(0.2, 0.2, 0.2);

// ===== PROCEDURAL GENERATION =====
// Hash function for pseudo-random numbers
float hash(float seed) {
    return frac(sin(seed * 12.9898) * 43758.5453);
}

float hash2D(float2 seed) {
    return frac(sin(dot(seed, float2(12.9898, 78.233))) * 43758.5453);
}

// Get procedural color based on ID
float3 getProceduralColor(uint lightId) {
    // Generate a spectrum of colors across hue space
    float hue = hash(float(lightId) * 0.17) * 6.0;
    
    // Simple RGB from HSV conversion
    float x = 1.0 - abs(fmod(hue, 2.0) - 1.0);
    
    float3 color;
    if (hue < 1.0) color = float3(1.0, x, 0.0);
    else if (hue < 2.0) color = float3(x, 1.0, 0.0);
    else if (hue < 3.0) color = float3(0.0, 1.0, x);
    else if (hue < 4.0) color = float3(0.0, x, 1.0);
    else if (hue < 5.0) color = float3(x, 0.0, 1.0);
    else color = float3(1.0, 0.0, x);
    
    // Add some brightness variation
    float brightness = 0.7 + hash(float(lightId) * 0.41) * 0.3;
    
    return color * brightness;
}

// Calculate procedural light position based on ID and time
uint2 getProceduralLightPosition(uint lightId, float time) {
    // Generate a base position using the light ID
    float baseX = hash(float(lightId) * 0.13) * WORLD_SIDE_LENGTH;
    float baseY = hash(float(lightId) * 0.29) * WORLD_SIDE_LENGTH;
    
    // Generate motion parameters
    float speed = 0.2 + hash(float(lightId) * 0.71) * 0.8;
    float radius = 5.0 + hash(float(lightId) * 0.51) * 35.0;
    
    // Motion pattern based on light ID modulo 6
    float2 offset = float2(0, 0);
    
    switch (lightId % 6) {
        case 0: // Circular motion
            offset.x = radius * cos(time * speed);
            offset.y = radius * sin(time * speed);
            break;
            
        case 1: // Horizontal sine wave
            offset.x = radius * sin(time * speed);
            break;
            
        case 2: // Vertical sine wave
            offset.y = radius * sin(time * speed * 0.8);
            break;
            
        case 3: // Diagonal motion
            offset.x = radius * 0.7 * sin(time * speed * 0.7);
            offset.y = radius * 0.7 * cos(time * speed);
            break;
            
        case 4: // Spiral motion
            float spiralPhase = time * speed * 0.5;
            float spiralRadius = (radius * 0.5) + (radius * 0.5) * sin(spiralPhase * 0.3);
            offset.x = spiralRadius * cos(spiralPhase);
            offset.y = spiralRadius * sin(spiralPhase);
            break;
            
        case 5: // Bouncing motion
            float bounceFactor = (sin(time * speed * 0.5) + 1.0) * 0.5;
            bounceFactor = bounceFactor * bounceFactor;
            offset.y = radius * bounceFactor;
            break;
    }
    
    // Apply offset to base position and clamp to world boundaries
    float finalX = clamp(baseX + offset.x, 5.0, float(WORLD_SIDE_LENGTH) - 5.0);
    float finalY = clamp(baseY + offset.y, 5.0, float(WORLD_SIDE_LENGTH) - 5.0);
    
    return uint2(uint(finalX), uint(finalY));
}

// ===== OCCLUSION SYSTEM =====
// Determine if a cell is an occluder (wall) based on its position
bool isOccluder(uint2 pos) {
    // Pattern 0: No occluders
    if (OCCLUDER_PATTERN == 0) {
        return false;
    }
    
    // Pattern 1: Perimeter walls with some interior walls
    else if (OCCLUDER_PATTERN == 1) {
        // Thicker perimeter walls (2 pixels wide)
        if (pos.x <= 1 || pos.y <= 1 || pos.x >= WORLD_SIDE_LENGTH-2 || pos.y >= WORLD_SIDE_LENGTH-2)
            return true;
            
        // Some vertical walls - FIXED FOR SLANG - thicker now
        uint wallPos1 = WORLD_SIDE_LENGTH / 4;
        uint wall1TopLimit = (WORLD_SIDE_LENGTH * 2) / 3;
        uint wall1BottomLimit = WORLD_SIDE_LENGTH / 8;
        
        if ((pos.x == wallPos1 || pos.x == wallPos1+1) && pos.y < wall1TopLimit && pos.y > wall1BottomLimit)
            return true;
            
        uint wallPos2 = (WORLD_SIDE_LENGTH * 3) / 4;  
        uint wall2TopLimit = (WORLD_SIDE_LENGTH * 7) / 8;
        uint wall2BottomLimit = WORLD_SIDE_LENGTH / 3;
        
        if ((pos.x == wallPos2 || pos.x == wallPos2+1) && pos.y > wall2BottomLimit && pos.y < wall2TopLimit)
            return true;
            
        // Some horizontal walls - FIXED FOR SLANG - thicker now
        uint hWallPos1 = WORLD_SIDE_LENGTH / 4;
        uint hWall1RightLimit = (WORLD_SIDE_LENGTH * 3) / 4;
        uint hWall1LeftLimit = WORLD_SIDE_LENGTH / 3;
        
        if ((pos.y == hWallPos1 || pos.y == hWallPos1+1) && pos.x > hWall1LeftLimit && pos.x < hWall1RightLimit)
            return true;
            
        uint hWallPos2 = (WORLD_SIDE_LENGTH * 3) / 5;
        uint hWall2RightLimit = (WORLD_SIDE_LENGTH * 2) / 3;
        uint hWall2LeftLimit = WORLD_SIDE_LENGTH / 6;
        
        if ((pos.y == hWallPos2 || pos.y == hWallPos2+1) && pos.x < hWall2RightLimit && pos.x > hWall2LeftLimit)
            return true;
            
        return false;
    }
    
    // Pattern 2: Maze-like pattern
    else if (OCCLUDER_PATTERN == 2) {
        // Create a maze-like structure using a cellular pattern
        
        // Grid size for the maze
        uint MAZE_CELL_SIZE = 16;
        
        // Convert to maze grid coordinates
        uint mazeX = pos.x / MAZE_CELL_SIZE;
        uint mazeY = pos.y / MAZE_CELL_SIZE;
        
        // Local coordinates within the maze cell
        uint localX = pos.x % MAZE_CELL_SIZE;
        uint localY = pos.y % MAZE_CELL_SIZE;
        
        // Cell hash value for maze pattern
        float cellNoise = hash2D(float2(mazeX, mazeY));
        
        // Create walls - every maze cell has walls based on its hash
        // Make walls 2 pixels thick
        
        // Vertical wall (if cell hash > 0.5)
        if (cellNoise > 0.5 && (localX == 0 || localX == 1))
            return true;
            
        // Horizontal wall (if cell hash < 0.5)
        if (cellNoise < 0.5 && (localY == 0 || localY == 1))
            return true;
            
        // Additional diagonal walls (if cell hash in mid range)
        if (cellNoise > 0.3 && cellNoise < 0.7 && (localX == localY || localX == localY+1 || localX+1 == localY))
            return true;
            
        // Always have perimeter walls - 2 pixels thick
        if (pos.x <= 1 || pos.y <= 1 || pos.x >= WORLD_SIDE_LENGTH-2 || pos.y >= WORLD_SIDE_LENGTH-2)
            return true;
            
        return false;
    }
    
    // Pattern 3: Random blocks
    else if (OCCLUDER_PATTERN == 3) {
        // Deterministic random blocks
        float cellRand = hash2D(float2(pos.x * 0.67, pos.y * 0.67));
        
        // Create some larger block structures
        uint blockSize = 4; // Ensure blocks are at least 2x2
        uint blockX = pos.x / blockSize;
        uint blockY = pos.y / blockSize;
        
        // Hash for this block
        float blockNoise = hash2D(float2(blockX, blockY));
        
        // Make some blocks solid
        if (blockNoise < OCCLUDER_DENSITY)
            return true;
            
        // Plus perimeter walls - thicker
        if (pos.x <= 1 || pos.y <= 1 || pos.x >= WORLD_SIDE_LENGTH-2 || pos.y >= WORLD_SIDE_LENGTH-2)
            return true;
            
        return false;
    }
    
    return false;
}

// Check if cell is adjacent to an occluder but not an occluder itself
bool isAirAdjacentToOccluder(uint2 pos) {
    // If the cell is an occluder, it's not what we're looking for
    if (isOccluder(pos)) return false;
    
    // Check all 8 surrounding cells
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue;
            
            int nx = int(pos.x) + dx;
            int ny = int(pos.y) + dy;
            
            // Bounds check
            if (nx < 0 || nx >= WORLD_SIDE_LENGTH || ny < 0 || ny >= WORLD_SIDE_LENGTH)
                continue;
                
            // If any neighbor is an occluder, this cell is adjacent to an occluder
            if (isOccluder(uint2(nx, ny)))
                return true;
        }
    }
    
    return false;
}

// Check if there's a wall blocking diagonal propagation
bool isDiagonalBlocked(uint2 currentPos, uint2 neighborPos) {
    // Only check diagonal neighbors
    bool isDiagonal = (currentPos.x != neighborPos.x) && (currentPos.y != neighborPos.y);
    if (!isDiagonal) return false;
    
    // For diagonal neighbors, check if there are occluders at the corner cells
    uint2 corner1 = uint2(currentPos.x, neighborPos.y);
    uint2 corner2 = uint2(neighborPos.x, currentPos.y);
    
    // If either corner is an occluder, the diagonal is blocked
    return isOccluder(corner1) || isOccluder(corner2);
}

// Helper function to propagate light (but not render it) through air cells
float3 propagateLightThroughCell(uint2 currentPos, uint2 neighborPos) {
    uint index = neighborPos.y * WORLD_SIDE_LENGTH + neighborPos.x;
    
    // Check if this position is an occluder
    if (isOccluder(neighborPos)) {
        return float3(0, 0, 0); // Occluders block light propagation
    }
    
    // Check for diagonal blocking
    if (isDiagonalBlocked(currentPos, neighborPos)) {
        return float3(0, 0, 0);
    }
    
    // Otherwise return the actual light value
    return p.lightingBuffer.light[index];
}

[numthreads(8, 8, 1)]
void main(uint3 threadId: SV_DispatchThreadID) {
    // Calculate the position in the grid
    uint x = threadId.x;
    uint y = threadId.y;
    
    // Check if we're within bounds of the world
    if (x >= WORLD_SIDE_LENGTH || y >= WORLD_SIDE_LENGTH)
        return;
    
    // Calculate linear index into the lighting buffer
    uint index = y * WORLD_SIDE_LENGTH + x;
    uint2 cellPos = uint2(x, y);
    
    // Time value for animations
    float time = p.time * 0.5;
    
    // ===== Check if this cell is an emissive light source =====
    bool isEmissive = false;
    float3 emissiveColor = float3(0, 0, 0);
    
    // Generate all light positions procedurally
    for (uint i = 0; i < LIGHT_COUNT; i++) {
        uint2 lightPos = getProceduralLightPosition(i, time);
        
        // Skip if this light would be inside an occluder
        if (isOccluder(lightPos))
            continue;
            
        if (x == lightPos.x && y == lightPos.y) {
            emissiveColor = getProceduralColor(i) * EMISSION_STRENGTH;
            isEmissive = true;
            break;
        }
    }
    
    // ===== FIRST PASS: PROPAGATE LIGHT IN AIR (invisibly) =====
    // This is the invisible light energy that propagates through air
    float3 propagatedLight = float3(0, 0, 0);
    
    if (isEmissive) {
        // Light sources emit full brightness into the invisible propagation buffer
        propagatedLight = emissiveColor;
    } else if (!isOccluder(cellPos)) {
        // Only propagate through air cells
        uint2 neighborPos[8];
        float weights[8] = {1.0, 0.7, 1.0, 0.7, 1.0, 0.7, 1.0, 0.7};
        
        // Define neighbor positions
        neighborPos[0] = uint2(x, y-1);     // Top
        neighborPos[1] = uint2(x+1, y-1);   // Top-right
        neighborPos[2] = uint2(x+1, y);     // Right
        neighborPos[3] = uint2(x+1, y+1);   // Bottom-right
        neighborPos[4] = uint2(x, y+1);     // Bottom
        neighborPos[5] = uint2(x-1, y+1);   // Bottom-left
        neighborPos[6] = uint2(x-1, y);     // Left
        neighborPos[7] = uint2(x-1, y-1);   // Top-left
        
        // Calculate max and weighted average
        float3 maxNeighborLight = float3(0, 0, 0);
        float3 avgNeighborLight = float3(0, 0, 0);
        float totalWeight = 0.0;
        
        // Process each neighbor that's in bounds
        for (uint n = 0; n < 8; n++) {
            // Check if neighbor is in bounds
            bool inBounds = true;
            if (n == 0 && y == 0) inBounds = false;
            if (n == 1 && (y == 0 || x == WORLD_SIDE_LENGTH-1)) inBounds = false;
            if (n == 2 && x == WORLD_SIDE_LENGTH-1) inBounds = false;
            if (n == 3 && (y == WORLD_SIDE_LENGTH-1 || x == WORLD_SIDE_LENGTH-1)) inBounds = false;
            if (n == 4 && y == WORLD_SIDE_LENGTH-1) inBounds = false;
            if (n == 5 && (y == WORLD_SIDE_LENGTH-1 || x == 0)) inBounds = false;
            if (n == 6 && x == 0) inBounds = false;
            if (n == 7 && (y == 0 || x == 0)) inBounds = false;
            
            if (inBounds) {
                // Using our propagation function
                float3 neighborLight = propagateLightThroughCell(cellPos, neighborPos[n]);
                
                float neighborIntensity = length(neighborLight);
                float maxIntensity = length(maxNeighborLight);
                
                if (neighborIntensity * weights[n] > maxIntensity) {
                    maxNeighborLight = neighborLight;
                }
                
                // Only add to average if there is actual light
                if (neighborIntensity > 0.0) {
                    avgNeighborLight += neighborLight * weights[n];
                    totalWeight += weights[n];
                }
            }
        }
        
        if (totalWeight > 0.0) {
            avgNeighborLight /= totalWeight;
        }
        
        // Blend max and average for air propagation
        float3 blendedLight = lerp(maxNeighborLight, avgNeighborLight, SMOOTH_BLEND_WEIGHT);
        
        // Apply propagation factor - this is the invisible propagating light
        propagatedLight = blendedLight * SMOOTH_PROP_FACTOR;
    }
    
    // ===== FINAL VISUALS: DETERMINE WHAT GETS RENDERED =====
    float3 visibleColor;
    
    if (isEmissive) {
        // Light sources are always visible at full brightness
        visibleColor = emissiveColor;
    }
    else if (isOccluder(cellPos)) {
        // Occluders are visible with their ambient color plus any light from adjacent air cells
        
        // Get the max light from adjacent air cells - this is what hits the occluder surface
        float3 lightHittingOccluder = float3(0, 0, 0);
        
        // Sample light from surrounding air cells
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) continue;
                
                int nx = int(x) + dx;
                int ny = int(y) + dy;
                
                // Bounds check
                if (nx < 0 || nx >= WORLD_SIDE_LENGTH || ny < 0 || ny >= WORLD_SIDE_LENGTH)
                    continue;
                    
                uint2 neighborPos = uint2(nx, ny);
                
                // Only consider non-occluder neighbors (air cells)
                if (!isOccluder(neighborPos)) {
                    // Get the light value from this air cell
                    uint neighborIndex = ny * WORLD_SIDE_LENGTH + nx;
                    float3 neighborLight = p.lightingBuffer.light[neighborIndex];
                    
                    // Take the brightest light hitting this occluder
                    if (length(neighborLight) > length(lightHittingOccluder)) {
                        lightHittingOccluder = neighborLight;
                    }
                }
            }
        }
        
        // Blend the occluder's ambient color with the light hitting it
        visibleColor = OCCLUDER_COLOR + lightHittingOccluder * 0.8; // Adjust blend factor as needed
    }
    else {
        // Air cells are invisible - just black
        visibleColor = float3(0, 0, 0);
    }
    
    // Write the propagated light (invisible) to the buffer for the next frame's propagation
    p.lightingBuffer.light[index] = propagatedLight;
    
    // Write the visible color to the output buffer
    p.renderBuffer.color[index] = visibleColor;
}