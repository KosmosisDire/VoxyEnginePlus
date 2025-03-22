#include "trace.slang"

//--------- Helper Functions ---------//

// Ray-plane intersection: returns distance along ray to intersection or -1 if no hit
float RayPlaneIntersection(float3 rayOrigin, float3 rayDir, float3 planeNormal, float3 planePoint) {
    float denom = dot(rayDir, planeNormal);
    if (abs(denom) < 0.0001)
        return -1.0; // No intersection (ray parallel to plane)
    
    float t = dot(planePoint - rayOrigin, planeNormal) / denom;
    return t > 0.0 ? t : -1.0; // Return positive intersection or -1
}

// Anti-aliased grid line: returns 0-1 value where 0 = on grid line, 1 = far from line
float GetGridLine(float position, float cellSize, float lineWidth) {
    float normPos = position / cellSize;
    float fractPos = abs(frac(normPos) - 0.5) * 2.0;
    return smoothstep(lineWidth, lineWidth * 2.0, fractPos);
}

// Get combined grid pattern (regular + major): returns 0-1 where 0 = on a line
float GetGrid(float2 position, float cellSize, float lineWidth, float majorLineWidth, float majorGridScale) {
    // Regular grid
    float2 gridLines = float2(
        GetGridLine(position.x, cellSize, lineWidth),
        GetGridLine(position.y, cellSize, lineWidth)
    );
    
    // Major grid
    float2 majorGridLines = float2(
        GetGridLine(position.x, cellSize * majorGridScale, majorLineWidth),
        GetGridLine(position.y, cellSize * majorGridScale, majorLineWidth)
    );
    
    // Combine grids - min value means visible line
    return min(min(gridLines.x, gridLines.y), min(majorGridLines.x, majorGridLines.y));
}

// Get axis line: returns 0-1 where 0 = on axis, 1 = far from axis
float GetAxis(float coordinate, float width) {
    return smoothstep(width, width * 2.0, abs(coordinate));
}

// Distance-based fade: returns 1 at start fading to 0 at end
float GetDistanceFade(float distance, float startFade, float endFade) {
    return 1.0 - smoothstep(startFade, endFade, distance);
}

// Ray-cylinder intersection for vertical axis
bool RayCylinderIntersect(float3 rayOrigin, float3 rayDir, float3 cylinderStart, float3 cylinderDir, 
                          float cylinderRadius, out float t, out float3 hitPoint) {
    float3 oc = rayOrigin - cylinderStart;
    float a = dot(rayDir, rayDir) - pow(dot(rayDir, cylinderDir), 2);
    float b = 2.0 * (dot(rayDir, oc) - dot(rayDir, cylinderDir) * dot(oc, cylinderDir));
    float c = dot(oc, oc) - pow(dot(oc, cylinderDir), 2) - cylinderRadius * cylinderRadius;
    
    float discriminant = b * b - 4.0 * a * c;
    
    if (discriminant < 0) {
        t = -1.0;
        hitPoint = float3(0,0,0);
        return false;
    }
    
    t = (-b - sqrt(discriminant)) / (2.0 * a);
    
    if (t <= 0) {
        t = -1.0;
        hitPoint = float3(0,0,0);
        return false;
    }
    
    hitPoint = rayOrigin + t * rayDir;
    return true;
}

//--------- Drawing Functions ---------//

// Draw background sky gradient
float4 DrawBackground(float3 rayDir) {
    float gradient = 0.5 + 0.5 * rayDir.y;
    float3 skyColor = lerp(float3(0.05, 0.05, 0.07), float3(0.1, 0.12, 0.15), gradient);
    return float4(skyColor, 1.0);
}

// Draw grid and axes
float4 DrawGridAndAxes(float3 rayOrigin, float3 rayDir, float4 backgroundColor) {
    // Grid and visualization parameters
    static const float CELL_SIZE = 1.0;
    static const float LINE_WIDTH = 0.01;
    static const float MAJOR_LINE_WIDTH = 0.01;
    static const float MAJOR_GRID_SCALE = 5.0;
    static const float MAX_GRID_DIST = 50.0;
    static const float FADE_START = MAX_GRID_DIST * 0.5;
    static const float AXIS_WIDTH = 0.04;
    static const float Y_AXIS_RADIUS = 0.04;
    static const float Y_AXIS_MAX_HEIGHT = 10.0;
    static const float Y_AXIS_FADE_START = 5.0;
    static const float3 GRID_COLOR = float3(0.3, 0.3, 0.35);
    static const float3 X_AXIS_COLOR = float3(0.9, 0.2, 0.2);  // Red
    static const float3 Y_AXIS_COLOR = float3(0.2, 0.9, 0.2);  // Green
    static const float3 Z_AXIS_COLOR = float3(0.2, 0.4, 0.9);  // Blue
    
    float4 finalColor = backgroundColor;
    
    // Ground plane parameters
    float3 planeNormal = float3(0, 1, 0);
    float3 planePoint = float3(0, 0, 0);
    
    // Intersect with ground plane
    float t_ground = RayPlaneIntersection(rayOrigin, rayDir, planeNormal, planePoint);
    
    if (t_ground > 0) {
        float3 hitPoint = rayOrigin + t_ground * rayDir;
        
        // Distance from center of grid
        float distFromCenter = length(float2(hitPoint.x, hitPoint.z));
        
        if (distFromCenter <= MAX_GRID_DIST) {
            // Calculate fade factor based on distance
            float fadeFactor = GetDistanceFade(distFromCenter, FADE_START, MAX_GRID_DIST);
            
            // Get grid line intensity (lower values = visible lines)
            float gridValue = GetGrid(float2(hitPoint.x, hitPoint.z), CELL_SIZE, 
                                     LINE_WIDTH, MAJOR_LINE_WIDTH, MAJOR_GRID_SCALE);
            
            // Get axis intensities (lower values = visible lines)
            float xAxisValue = GetAxis(hitPoint.z, AXIS_WIDTH);
            float zAxisValue = GetAxis(hitPoint.x, AXIS_WIDTH);
            
            // Track if we're rendering any grid element
            bool renderingGrid = false;
            float3 gridElementColor = float3(0, 0, 0);
            float gridElementAlpha = 0;
            
            // Only render the actual grid lines, not the space between
            if (gridValue < 0.5) {
                // Calculate intensity of the grid line
                float gridIntensity = (1.0 - gridValue * 2.0) * fadeFactor;
                gridElementColor = GRID_COLOR;
                gridElementAlpha = gridIntensity * 0.6;
                renderingGrid = true;
            }
            
            // Apply X axis (red)
            if (xAxisValue < 0.5) {
                float xAxisIntensity = (1.0 - xAxisValue * 2.0) * fadeFactor;
                gridElementColor = X_AXIS_COLOR;
                gridElementAlpha = xAxisIntensity * 0.8;
                renderingGrid = true;
            }
            
            // Apply Z axis (blue)
            if (zAxisValue < 0.5) {
                float zAxisIntensity = (1.0 - zAxisValue * 2.0) * fadeFactor;
                // If we're at the intersection of X and Z axes, blend the colors
                if (xAxisValue < 0.5) {
                    gridElementColor = lerp(gridElementColor, Z_AXIS_COLOR, 0.5);
                    gridElementAlpha = max(gridElementAlpha, zAxisIntensity * 0.8);
                } else {
                    gridElementColor = Z_AXIS_COLOR;
                    gridElementAlpha = zAxisIntensity * 0.8;
                }
                renderingGrid = true;
            }
            
            // Only apply grid elements if we're actually rendering one
            if (renderingGrid) {
                // Blend the grid element with the sky using the calculated alpha
                finalColor.rgb = lerp(finalColor.rgb, gridElementColor, gridElementAlpha);
            }
        }
    }
    
    return finalColor;
}

// Draw voxels
float4 DrawVoxels(float3 rayOrigin, float3 rayDir, float4 sceneColor) {
    float3 sunDir = normalize(float3(0.45, 0.4, 0.2));
    var hit = push.voxelMap.RayCast(int3(0, 0, 0), 4, rayOrigin, rayDir, false);
    
    if (!hit.Miss) {
        float4 voxelColor = float4(0, 0, 0, 1);
        voxelColor.rgb = dot(hit.Normal, sunDir) * 0.5 + 0.5;

        // Cast towards the sun for shadows
        var sunHit = push.voxelMap.RayCast(0, 4, hit.Pos + hit.Normal * 0.00001, sunDir, false);
        if (!sunHit.Miss) {
            voxelColor.rgb *= 0.5; // Apply shadow
        }
        
        // Completely replace the scene color with voxel color (no blending)
        return voxelColor;
    }
    
    return sceneColor;
}

//--------- Main Function ---------//

[numthreads(16, 16, 1)]
void main(uint2 pixel_i: SV_DispatchThreadID) {
    RWTexture2D<float4> screenTex = RWTexture2D<float4>::get(push.screen);
    
    // Get ray for this pixel
    float3 rayOrigin, rayDir;
    getRayFromPixel(pixel_i, push.frame_dim, rayOrigin, rayDir);
    
    // Draw background
    float4 color = DrawBackground(rayDir);
    
    // Draw grid and axes
    color = DrawGridAndAxes(rayOrigin, rayDir, color);
    
    // Draw voxels
    color = DrawVoxels(rayOrigin, rayDir, color);
    
    // Write final color to screen
    screenTex[pixel_i] = color;
}