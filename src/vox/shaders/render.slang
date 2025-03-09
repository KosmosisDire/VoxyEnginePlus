#include "shared.inl"

[[vk::push_constant]]
const ComputePush p;

// Hash function for pseudo-random number generation
float hash(float2 p)
{
    float3 p3 = frac(float3(p.xyx) * float3(443.897, 441.423, 437.195));
    p3 += dot(p3, p3.yzx + 19.19);
    return frac((p3.x + p3.y) * p3.z);
}

// Get random color based on grid cell coordinates
float3 get_random_color(uint2 grid_cell)
{
    float h1 = hash(float2(grid_cell.x, grid_cell.y));
    float h2 = hash(float2(grid_cell.x, grid_cell.y * 42.5));
    float h3 = hash(float2(grid_cell.x * 17.3, grid_cell.y));
    
    return float3(h1, h2, h3);
}

[numthreads(8, 8, 1)]
void main(uint3 pixel_i: SV_DispatchThreadID)
{
    uint2 pixel = pixel_i.xy;
    if (pixel.x >= p.frame_dim.x || pixel.y >= p.frame_dim.y)
        return;
    
    RWTexture2D<float4> screenTex = RWTexture2D<float4>::get(p.screen);
    
    // Default to black background color
    float4 color = float4(0, 0, 0, 1);
    
    // Calculate cell size based on smaller dimension to fit entire grid
    uint cell_size = min(p.frame_dim.x / WORLD_SIDE_LENGTH, p.frame_dim.y / WORLD_SIDE_LENGTH);
    
    // If cell size would be zero, set minimum of 1 pixel
    cell_size = max(cell_size, 1);
    
    // Calculate total grid width and height
    uint grid_width = cell_size * WORLD_SIDE_LENGTH;
    uint grid_height = cell_size * WORLD_SIDE_LENGTH;
    
    // Calculate offset to center the grid
    uint offset_x = (p.frame_dim.x - grid_width) / 2;
    uint offset_y = (p.frame_dim.y - grid_height) / 2;
    
    // Check if pixel is within the grid bounds
    if (pixel.x >= offset_x && pixel.x < offset_x + grid_width &&
        pixel.y >= offset_y && pixel.y < offset_y + grid_height)
    {
        // Calculate the grid cell this pixel belongs to
        uint2 grid_cell = uint2(
            (pixel.x - offset_x) / cell_size,
            (pixel.y - offset_y) / cell_size
        );
        
        // Get color from the render buffer instead of lighting buffer
        color.rgb = p.renderBuffer.color[grid_cell.y * WORLD_SIDE_LENGTH + grid_cell.x];
        color.rgb += p.lightingBuffer.light[grid_cell.y * WORLD_SIDE_LENGTH + grid_cell.x] * 0.15;
    }
    
    screenTex[pixel] = color;
}