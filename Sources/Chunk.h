//
// Created by sunvy on 31/05/2026.
//

#pragma once

#include "BlockRegistry.h"

#define SIZE_X 16
#define SIZE_Y 255
#define SIZE_Z 16

namespace Sunset
{
    class Drawable;
}

class Chunk
{
public:
    explicit Chunk(const glm::vec2& position);

    ~Chunk();

    void Draw() const;

    void BuildMesh();

    glm::ivec2 m_Position;
    std::array<Block, SIZE_X * SIZE_Y * SIZE_Z> Blocks;
    std::vector<float> NoiseValue;
    std::unique_ptr<Sunset::Drawable> m_Drawable;
};
