//
// Created by sunvy on 31/05/2026.
//

#include "Chunk.h"

#include "BaseObject/BaseCube.h"
#include "Render/Drawable.h"
#include "Render/RenderCommande.h"
#include "Render/Meshes/Mesh.h"

namespace
{
    constexpr std::array<glm::ivec3, 6> checkDir = {
        glm::ivec3{-1, 0, 0},
        {1, 0, 0},
        {0, -1, 0},
        {0, 1, 0},
        {0, 0, -1},
        {0, 0, 1}
    };

    constexpr int ChunkHeight = SIZE_Y * 2;
    constexpr int SliceSize = SIZE_X * SIZE_Z;

    constexpr bool IsInsideChunk(const int x, const int y, const int z)
    {
        return x >= 0 && x < SIZE_X
            && y >= -SIZE_Y && y < SIZE_Y
            && z >= 0 && z < SIZE_Z;
    }

    constexpr size_t BlockIndex(const int x, const int y, const int z)
    {
        return static_cast<size_t>(x + z * SIZE_X + (y + SIZE_Y) * SliceSize);
    }

    bool IsAir(const Chunk& chunk, const int x, const int y, const int z)
    {
        return chunk.Blocks[BlockIndex(x, y, z)] == BlockRegistry::AIR;
    }
}

Chunk::Chunk(const glm::vec2 &position)
    : m_Position(position)
    , m_Drawable(std::make_unique<Sunset::Drawable>())
    , Blocks()
{
}

Chunk::~Chunk()
{
}

void Chunk::Draw() const
{
    Sunset::RenderCommande::Submit(*m_Drawable);

    // for (int x = 0; x < SIZE_X; ++x)
    // {
    //     for (int z = 0; z < SIZE_Z; ++z)
    //     {
    //         for (int y = -SIZE_Y; y < SIZE_Y; ++y)
    //         {
    //             if (Blocks[BlockIndex(x, y, z)] != BlockRegistry::STONE)
    //                 continue;
    //
    //             for (const auto& dir : checkDir)
    //             {
    //                 if (IsInsideChunk(x + dir.x, y + dir.y, z + dir.z))
    //                 {
    //                     if (IsAir(*this, x + dir.x, y + dir.y, z + dir.z))
    //                     {
    //                         Sunset::DrawCube({x + m_Position.x * SIZE_X, y, z + m_Position.y * SIZE_Z}, {}, {});
    //                         break;
    //                     }
    //                 }
    //             }
    //         }
    //     }
    // }
}
