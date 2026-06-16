//
// Created by sunvy on 15/06/2026.
//

#pragma once

#include "../Chunk.h"

struct GenerationData
{
    int seed = 0;
};

struct GeneratedChunk
{
    glm::ivec2 position;
    std::array<Block, SIZE_X * (SIZE_Y * 2) * SIZE_Z> blocks;
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
