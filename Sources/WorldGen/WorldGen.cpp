//
// Created by sunvy on 15/06/2026.
//

#include "WorldGen.h"

#include "CaveGen.h"
#include "HeightGen.h"
#include "../Chunk.h"
#include "../Noise.h"

namespace
{
    int Seed = 0;

    std::vector<std::unique_ptr<GenLayout>>& worldGenLayout()
    {
        static std::vector<std::unique_ptr<GenLayout>> WorldGenLayout;
        return WorldGenLayout;
    }
}

GeneratedChunk::GeneratedChunk(const glm::ivec2 pos)
    : position(pos)
{
    blocks.fill(BlockRegistry::AIR);
}

void WorldGen::Init(const int seed)
{
    LOG("SunsetCraft", trace, "WorldGen Init");
    Seed = seed;
    Noise::Init(Seed);

    worldGenLayout().emplace_back(std::make_unique<HeightGen>());
    // WorldGenLayout.emplace_back(std::make_unique<HaloGen>());
    // worldGenLayout().emplace_back(std::make_unique<CaveGen>());
}

void WorldGen::Destroy()
{
    LOG("SunsetCraft", trace, "WorldGen Destroy");
    worldGenLayout().clear();
    Noise::Destroy();
}

void WorldGen::GenChunk(GeneratedChunk &chunk)
{
    GenerationData data;
    data.seed = Seed;
    for (auto& layout : worldGenLayout())
    {
        (*layout)(chunk, data);
    }
}
