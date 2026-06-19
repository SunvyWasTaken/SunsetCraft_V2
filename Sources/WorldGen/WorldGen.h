//
// Created by sunvy on 15/06/2026.
//

#pragma once

#include "../Chunk.h"

struct NoiseData;

struct GenerationData
{
    int seed = 0;
    std::vector<float> NoiseValue;
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
