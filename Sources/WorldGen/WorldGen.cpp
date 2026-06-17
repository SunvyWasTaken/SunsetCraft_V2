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
    std::vector<std::unique_ptr<GenLayout>> WorldGenLayout;

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

    WorldGenLayout.emplace_back(std::make_unique<HeightGen>());
    // WorldGenLayout.emplace_back(std::make_unique<HaloGen>());
    WorldGenLayout.emplace_back(std::make_unique<CaveGen>());
    // todo : Continue Biome.
}

void WorldGen::Destroy()
{
    LOG("SunsetCraft", trace, "WorldGen Destroy");
    WorldGenLayout.clear();
    Noise::Destroy();
}

void WorldGen::GenChunk(GeneratedChunk &chunk)
{
    GenerationData data;
    data.seed = Seed;
    for (auto& layout : WorldGenLayout)
    {
        (*layout)(chunk, data);
    }
}
