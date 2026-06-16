//
// Created by sunvy on 15/06/2026.
//

#include "WorldGen.h"

#include "HaloGen.h"
#include "HeightGen.h"
#include "../Chunk.h"
#include "../Noise.h"
#include "Render/Drawable.h"
#include "Render/Material.h"
#include "Render/Shader.h"
#include "Render/BufferObject/Buffers.h"
#include "Render/Meshes/Mesh.h"

namespace
{
    int Seed = 0;
    std::vector<std::unique_ptr<GenLayout>> WorldGenLayout;
}

void WorldGen::Init(const int seed)
{
    LOG("SunsetCraft", trace, "WorldGen Init");
    Seed = seed;
    Noise::Init(Seed);

    WorldGenLayout.emplace_back(std::make_unique<HeightGen>());
    WorldGenLayout.emplace_back(std::make_unique<HaloGen>());
}

void WorldGen::Destroy()
{
    LOG("SunsetCraft", trace, "WorldGen Destroy");
    WorldGenLayout.clear();
    Noise::Destroy();
}

void WorldGen::GenChunk(Chunk &chunk)
{
    ChunkData data;
    data.seed = Seed;
    Noise::Get(data.NoiseValue, chunk.m_Position * glm::ivec2{SIZE_X, SIZE_Z});
    for (auto& layout : WorldGenLayout)
    {
        (*layout)(chunk, data);
    }
}
