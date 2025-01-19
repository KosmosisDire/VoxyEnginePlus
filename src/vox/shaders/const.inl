#pragma once
#include "daxa/daxa.inl"

static const daxa_u32 GRID_SIZE = 100;
static const daxa_u32 GRID_SIZE_SQUARE = GRID_SIZE * GRID_SIZE;
static const daxa_u32 GRID_SIZE_CUBE = GRID_SIZE * GRID_SIZE * GRID_SIZE;

static const daxa_u32 BRICK_SIZE = 6;
static const daxa_u32 BRICK_SIZE_SQUARE = BRICK_SIZE * BRICK_SIZE;
static const daxa_u32 BRICK_SIZE_CUBE = BRICK_SIZE * BRICK_SIZE * BRICK_SIZE;

static const daxa_u32 CHUNK_SIZE = 4;
static const daxa_u32 CHUNK_SIZE_SQUARE = CHUNK_SIZE * CHUNK_SIZE;
static const daxa_u32 CHUNK_SIZE_CUBE = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

static const daxa_f32 WORLD_VOXEL_SIZE = 1.0 / (BRICK_SIZE * CHUNK_SIZE);
static const daxa_f32 WORLD_BRICK_SIZE = 1.0 / CHUNK_SIZE;

static const daxa_u64 TOTAL_CHUNKS = GRID_SIZE_CUBE;
static const daxa_u64 TOTAL_BRICKS = TOTAL_CHUNKS * CHUNK_SIZE_CUBE;
static const daxa_u64 TOTAL_VOXELS = TOTAL_BRICKS * BRICK_SIZE_CUBE;


static const daxa_u32 BITS_PER_BYTE = 8;
static const float EPSILON = 0.0001;
static const float PI = 3.14159265359;
static const daxa_u32 MAX_UINT32 = 0xFFFFFFFF;
static const daxa_u64 MAX_UINT64 = 0xFFFFFFFFFFFFFFFF;