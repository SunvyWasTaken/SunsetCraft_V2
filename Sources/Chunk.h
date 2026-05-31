//
// Created by sunvy on 31/05/2026.
//

#pragma once

#include "BlockRegistry.h"

#define SIZE_X 100
#define SIZE_Y 75
#define SIZE_Z 100

class Chunk
{
public:
    explicit Chunk(const glm::vec2& position);

    ~Chunk();

    void Draw() const;

    glm::ivec2 m_Position;
    std::vector<float> NoiseValue;
};