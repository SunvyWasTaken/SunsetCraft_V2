//
// Created by sunvy on 31/05/2026.
//

#include "Chunk.h"

#include "BaseObject/BaseCube.h"
#include "Render/Drawable.h"
#include "Render/Meshes/Mesh.h"

Chunk::Chunk(const glm::vec2 &position)
    : m_Position(position)
    , Blocks()
    , NoiseValue()
{
}

Chunk::~Chunk()
{
}

void Chunk::Draw() const
{
    for (int x = 0; x < SIZE_X; ++x)
    {
        for (int z = 0; z < SIZE_Z; ++z)
        {
            const float val = NoiseValue[z + x * SIZE_X];
            for (int y = -SIZE_Y; y < SIZE_Y; ++y)
            {
                if (y == floor(val))
                {
                    Sunset::DrawCube({x + m_Position.x * SIZE_X, y, z + m_Position.y * SIZE_Z}, {}, {});
                }
            }
        }
    }
}

void Chunk::BuildMesh()
{
    std::vector<uint32_t> data;
    data.reserve(SIZE_X * SIZE_Y * SIZE_Z);
    for (int x = 0; x < SIZE_X; ++x)
    {
        for (int z = 0; z < SIZE_Z; ++z)
        {
            for (int y = -SIZE_Y; y < SIZE_Y; ++y)
            {
                size_t i = x + z * SIZE_X + y * SIZE_Y * SIZE_Z;
                if (Blocks[i] == BlockRegistry::STONE)
                {

                }
            }
        }
    }
    m_Drawable = std::make_unique<Sunset::Drawable>();
    m_Drawable->m_Mesh = Sunset::Mesh::CreateMesh(data.data(), sizeof(uint32_t), data.size(), {}, {});
}
