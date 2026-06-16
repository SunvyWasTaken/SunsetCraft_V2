//
// Created by sunvy on 15/06/2026.
//

#pragma once

class Chunk;

struct ChunkData
{
    std::vector<float> NoiseValue;
};

struct GenLayout
{
    virtual ~GenLayout() = default;
    virtual void operator()(Chunk& chunk, ChunkData& data) = 0;
};

struct WorldGen
{
    static void Init(int seed);
    static void Destroy();
    static void GenChunk(Chunk& chunk);
};
