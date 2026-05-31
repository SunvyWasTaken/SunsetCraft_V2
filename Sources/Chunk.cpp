//
// Created by sunvy on 31/05/2026.
//

#include "Chunk.h"

#include "BaseObject/BaseCube.h"

Chunk::Chunk(const glm::vec2 &position)
    : m_Position(position)
    , data()
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
            for (int y = 0; y < SIZE_Y; ++y)
            {
                // if (data[] == BlockRegistry::AIR)
                //     continue;
                // else (data[] == BlockRegistry::STONE)
                //     Sunset::DrawCube({x, y, z}, {}, {});
            }
        }
    }
}
