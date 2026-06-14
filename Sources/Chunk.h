//
// Created by sunvy on 31/05/2026.
//

#pragma once

#include "BlockRegistry.h"

#include <functional>

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
    struct BuildContext
    {
        std::vector<uint32_t> Vertices;
    };

    using BuildStep = std::function<void(Chunk&, BuildContext&)>;

    explicit Chunk(const glm::ivec2& position);

    ~Chunk();

    void Draw() const;

    void BuildMesh();

    void AddBuildStep(BuildStep step);
    void ResetBuildSteps();

    static uint32_t PackVertex(uint32_t x, uint32_t y, uint32_t z, uint32_t face, uint32_t corner, Block block);

    glm::ivec2 m_Position;
    std::array<Block, SIZE_X * SIZE_Y * SIZE_Z> Blocks;
    std::vector<float> NoiseValue;
    std::unique_ptr<Sunset::Drawable> m_Drawable;

private:
    std::vector<BuildStep> m_BuildSteps;
};
