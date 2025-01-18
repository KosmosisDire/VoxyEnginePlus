#include "common.slang"

// Hash function for gradient noise
uint hash(uint x)
{
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
}

// Generate pseudo-random gradient vector
float3 gradientDir(uint hash)
{
    // Use hash to generate unit vector
    float angle1 = float(hash & 255) / 256.0 * 2.0 * 3.14159;
    float angle2 = float((hash >> 8) & 255) / 256.0 * 3.14159;

    float sinA1 = sin(angle1);
    float cosA1 = cos(angle1);
    float sinA2 = sin(angle2);
    float cosA2 = cos(angle2);

    return float3(cosA1 * sinA2, sinA1 * sinA2, cosA2);
}

// Simple gradient noise implementation
float gradientNoise(float3 pos)
{
    // Get integer and fractional parts
    float3 i = floor(pos);
    float3 f = pos - i;

    // Smooth interpolation
    float3 u = f * f * (3.0 - 2.0 * f);

    // Generate corner hashes
    uint3 p = uint3(i);
    uint h000 = hash(p.x + hash(p.y + hash(p.z)));
    uint h001 = hash(p.x + hash(p.y + hash(p.z + 1)));
    uint h010 = hash(p.x + hash(p.y + 1 + hash(p.z)));
    uint h011 = hash(p.x + hash(p.y + 1 + hash(p.z + 1)));
    uint h100 = hash(p.x + 1 + hash(p.y + hash(p.z)));
    uint h101 = hash(p.x + 1 + hash(p.y + hash(p.z + 1)));
    uint h110 = hash(p.x + 1 + hash(p.y + 1 + hash(p.z)));
    uint h111 = hash(p.x + 1 + hash(p.y + 1 + hash(p.z + 1)));

    // Generate gradients and compute dot products
    float g000 = dot(gradientDir(h000), f);
    float g001 = dot(gradientDir(h001), float3(f.xy, f.z - 1));
    float g010 = dot(gradientDir(h010), float3(f.x, f.y - 1, f.z));
    float g011 = dot(gradientDir(h011), float3(f.x, f.y - 1, f.z - 1));
    float g100 = dot(gradientDir(h100), float3(f.x - 1, f.y, f.z));
    float g101 = dot(gradientDir(h101), float3(f.x - 1, f.y, f.z - 1));
    float g110 = dot(gradientDir(h110), float3(f.x - 1, f.y - 1, f.z));
    float g111 = dot(gradientDir(h111), float3(f.x - 1, f.y - 1, f.z - 1));

    // Trilinear interpolation of dot products
    return lerp(
        lerp(lerp(g000, g100, u.x), lerp(g010, g110, u.x), u.y),
        lerp(lerp(g001, g101, u.x), lerp(g011, g111, u.x), u.y),
        u.z);
}

[numthreads(8, 8, 8)]
void main(uint3 brickCoord: SV_DispatchThreadID)
{
    if (p.state_ptr.frame % 5 != 0)
        return;

    int worldBlockSpan = GRID_SIZE * CHUNK_SIZE;

    if (brickCoord.x >= worldBlockSpan || brickCoord.y >= worldBlockSpan || brickCoord.z >= worldBlockSpan)
        return;

    int brickIndex = brickCoord.x + brickCoord.y * worldBlockSpan + brickCoord.z * worldBlockSpan * worldBlockSpan;

    int3 chunkCoord = int3(brickCoord / CHUNK_SIZE);
    int chunkIndex = chunkCoord.x + chunkCoord.y * GRID_SIZE + chunkCoord.z * GRID_SIZE * GRID_SIZE;

    int localBrickIndex = brickCoord.x % CHUNK_SIZE + (brickCoord.y % CHUNK_SIZE) * CHUNK_SIZE + (brickCoord.z % CHUNK_SIZE) * CHUNK_SIZE_SQUARE;

    BrickBitmask brickOccupancy;
    brickOccupancy.part1 = 0;
    brickOccupancy.part2 = 0;
    brickOccupancy.compressed_data_ptr = 0;

    bool hasVoxel = false;
    
    // Iterate through each voxel in the brick using 3D nested loops
    for (uint z = 0; z < BRICK_SIZE; z++)
    {
        for (uint y = 0; y < BRICK_SIZE; y++)
        {
            for (uint x = 0; x < BRICK_SIZE; x++)
            {
                int3 localVoxelPos = int3(x, y, z);
                int3 absVoxel = brickCoord * BRICK_SIZE + localVoxelPos;
                
                // Debug: Print coordinates being used
                float3 noiseCoord = absVoxel * 0.002f + float3(p.state_ptr.time * 0.01f);
                float noiseValue = gradientNoise(noiseCoord);
                bool isSolid = noiseValue > -0.3f;

                int voxelIndex = voxelLocalIndex(localVoxelPos);

                if (isSolid)
                {
                    setBit(p.brick_occupancy_ptr, brickIndex, voxelIndex);
                    setBit(brickOccupancy, voxelIndex);
                    hasVoxel = true;
                }
                else
                {
                    clearBit(p.brick_occupancy_ptr, brickIndex, voxelIndex);
                    clearBit(brickOccupancy, voxelIndex);
                }
            }
        }
    }

    // p.brick_occupancy_ptr.occupancy[brickIndex] = brickOccupancy;

    if (hasVoxel)
    {
        setBit(p.chunk_occupancy_ptr, chunkIndex, localBrickIndex);
    }
    else
    {
        clearBit(p.chunk_occupancy_ptr, chunkIndex, localBrickIndex);
    }
}
