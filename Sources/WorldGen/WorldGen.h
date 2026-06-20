//
// Created by sunvy on 15/06/2026.
//

#pragma once

#include "../Chunk.h"

struct NoiseData;

inline size_t Index(int x, int y)
{
    return x + (y * SIZE_X);
}

inline size_t Index(int x, int y, int z)
{
    return x + (z * SIZE_X) + ((y + SIZE_Y) * SIZE_X * SIZE_Z);
}

struct GenerationData
{
    int seed = 0;
    std::vector<float> NoiseValue{};
};

struct GeneratedChunk
{
    glm::ivec2 position;
    std::array<BlockId, SIZE_X * (SIZE_Y * 2) * SIZE_Z> blocks;
    explicit GeneratedChunk(const glm::ivec2 pos);
};

struct GenLayout
{
    virtual ~GenLayout() = default;
    virtual void operator()(GeneratedChunk& chunk, GenerationData& data) = 0;
};

struct WorldGen
{
    static void Init(int seed);
    static void Destroy();
    static void GenChunk(GeneratedChunk& chunk);
};
