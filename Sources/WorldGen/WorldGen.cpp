//
// Created by sunvy on 15/06/2026.
//

#include "WorldGen.h"

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

    std::shared_ptr<Sunset::Shader> shader;

    constexpr std::array<glm::ivec3, 6> checkDir = {
        glm::ivec3{-1, 0, 0},
        {1, 0, 0},
        {0, -1, 0},
        {0, 1, 0},
        {0, 0, -1},
        {0, 0, 1}
    };

    bool IsInChunk(const int x, const int y, const int z)
    {
        return x <= 0 && x > SIZE_X && y <= -SIZE_Y && y > SIZE_Y && z <= 0 && z > SIZE_Z;
    }

    int GetIndex(const int x, const int y, const int z)
    {
        return x + z * SIZE_X + (y + SIZE_Y) * SIZE_X * SIZE_Z;
    }

    uint32_t EncodePoint(const int x, const int y, const int z, const int dir)
    {
        return   (static_cast<uint32_t>(x) & 0xFu)            |
                ((static_cast<uint32_t>(y) & 0x1FFu) << 4)     |
                ((static_cast<uint32_t>(z) & 0xFu) << 13)    |
                ((static_cast<uint32_t>(dir) & 0x7) << 16);
    }

    void CreateMesh(Chunk& chunk)
    {
        std::vector<uint32_t> points;
        for (int x = 0; x < SIZE_X; ++x)
        {
            for (int z = 0; z < SIZE_Z; ++z)
            {
                for (int y = -SIZE_Y; y < SIZE_Y; ++y)
                {
                    const int index = GetIndex(x, y, z);
                    if (chunk.Blocks[index] == BlockRegistry::STONE)
                    {
                        int idir = 0;
                        for (const auto& dir : checkDir)
                        {
                            ++idir;
                            if (!IsInChunk(x + dir.x, y + dir.y, z + dir.z))
                            {
                                points.emplace_back(EncodePoint(x, y, z, idir));
                                continue;
                            }
                            if (chunk.Blocks[GetIndex(x + dir.x, y + dir.y, z + dir.z)] == BlockRegistry::AIR)
                            {
                                points.emplace_back(EncodePoint(x + dir.x, y + dir.y, z + dir.z, idir));
                            }
                        }
                    }
                }
            }
        }

        std::vector<uint32_t> indices(points.size());
        chunk.m_Drawable->m_Mesh = Sunset::Mesh::CreateMesh(points.data(), sizeof(uint32_t), points.size(), indices, {Sunset::BufferElement(Sunset::ShaderDataType::UInt, "data")});
        chunk.m_Drawable->m_Position = {chunk.m_Position.x * SIZE_X, 0, chunk.m_Position.y * SIZE_Z};
        chunk.m_Drawable->m_RenderState.DrawInstance = true;
        chunk.m_Drawable->m_RenderState.nbrInstance = 6;
        chunk.m_Drawable->m_RenderState.HasIndice = false;
        if (shader == nullptr)
        {
            shader = std::make_shared<Sunset::Shader>(SHADERS_PATH "ChunkFragShader.frag", SHADERS_PATH "ChunkFragShader.vert");
        }
        chunk.m_Drawable->m_Material->m_Shader = shader;
    }
}

void WorldGen::Init(const int seed)
{
    LOG("SunsetCraft", trace, "WorldGen Init");
    Seed = seed;
    Noise::Init(Seed);

    WorldGenLayout.emplace_back(std::make_unique<HeightGen>());
}

void WorldGen::Destroy()
{
    LOG("SunsetCraft", trace, "WorldGen Destroy");
    shader.reset();
    WorldGenLayout.clear();
    Noise::Destroy();
}

void WorldGen::GenChunk(Chunk &chunk)
{
    ChunkData data;
    Noise::Get(data.NoiseValue, chunk.m_Position * glm::ivec2{SIZE_X, SIZE_Z});
    for (auto& layout : WorldGenLayout)
    {
        (*layout)(chunk, data);
    }
    CreateMesh(chunk);
}
