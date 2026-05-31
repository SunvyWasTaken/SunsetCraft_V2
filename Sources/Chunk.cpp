//
// Created by sunvy on 31/05/2026.
//

#include "Chunk.h"

#include "BaseObject/BaseCube.h"

Chunk::Chunk(const glm::vec2 &position)
    : m_Position(position)
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
                    Sunset::DrawCube({x + m_Position.x, y, z + m_Position.y}, {}, {});
                }
            }
        }
    }
}
