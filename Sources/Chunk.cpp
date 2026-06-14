//
// Created by sunvy on 31/05/2026.
//

#include "Chunk.h"

#include "BaseObject/BaseCube.h"
#include "Render/Drawable.h"
#include "Render/Material.h"
#include "Render/RenderCommande.h"
#include "Render/Shader.h"
#include "Render/Meshes/Mesh.h"

namespace
{
    constexpr glm::ivec3 FaceDirections[] = {
        { 0, 0, -1 },
        { 0, 0, 1 },
        { -1, 0, 0 },
        { 1, 0, 0 },
        { 0, 1, 0 },
        { 0, -1, 0 },
    };

    constexpr uint32_t FaceVertexCount = 6;

    std::shared_ptr<Sunset::Shader> shader = nullptr;

    size_t BlockIndex(const int x, const int y, const int z)
    {
        return static_cast<size_t>(x + z * SIZE_X + y * SIZE_X * SIZE_Z);
    }

    bool IsInsideChunk(const int x, const int y, const int z)
    {
        return x >= 0 && x < SIZE_X && y >= 0 && y < SIZE_Y && z >= 0 && z < SIZE_Z;
    }

    Block GetBlock(const Chunk& chunk, const int x, const int y, const int z)
    {
        if (!IsInsideChunk(x, y, z))
            return BlockRegistry::AIR;

        return chunk.Blocks[BlockIndex(x, y, z)];
    }

    bool IsFaceVisible(const Chunk& chunk, const int x, const int y, const int z, const glm::ivec3& direction)
    {
        return GetBlock(chunk, x + direction.x, y + direction.y, z + direction.z) == BlockRegistry::AIR;
    }

    void AppendVisibleFaces(Chunk& chunk, Chunk::BuildContext& context)
    {
        context.Vertices.reserve(SIZE_X * SIZE_Y * SIZE_Z);

        for (int y = 0; y < SIZE_Y; ++y)
        {
            for (int z = 0; z < SIZE_Z; ++z)
            {
                for (int x = 0; x < SIZE_X; ++x)
                {
                    const Block block = GetBlock(chunk, x, y, z);
                    if (block == BlockRegistry::AIR)
                        continue;

                    for (uint32_t face = 0; face < std::size(FaceDirections); ++face)
                    {
                        if (!IsFaceVisible(chunk, x, y, z, FaceDirections[face]))
                            continue;

                        for (uint32_t corner = 0; corner < FaceVertexCount; ++corner)
                        {
                            context.Vertices.push_back(Chunk::PackVertex(
                                static_cast<uint32_t>(x),
                                static_cast<uint32_t>(y),
                                static_cast<uint32_t>(z),
                                face,
                                corner,
                                block));
                        }
                    }
                }
            }
        }
    }
}

Chunk::Chunk(const glm::ivec2 &position)
    : m_Position(position)
    , Blocks()
    , NoiseValue()
    , m_BuildSteps()
{
    ResetBuildSteps();
}

Chunk::~Chunk()
{
}

void Chunk::Draw() const
{
    if (m_Drawable != nullptr)
        Sunset::RenderCommande::Submit(*m_Drawable);
}

void Chunk::BuildMesh()
{
    BuildContext context;

    for (const BuildStep& step : m_BuildSteps)
    {
        step(*this, context);
    }

    m_Drawable = std::make_unique<Sunset::Drawable>();
    m_Drawable->m_Mesh = Sunset::Mesh::CreateMesh(context.Vertices.data(), sizeof(uint32_t), context.Vertices.size(), {}, {});
    if (shader == nullptr)
    {
        shader = std::make_shared<Sunset::Shader>(SHADERS_PATH "shader.vert", SHADERS_PATH "shader.frag");
    }
    m_Drawable->m_Material->m_Shader = shader;
}

void Chunk::AddBuildStep(BuildStep step)
{
    m_BuildSteps.push_back(std::move(step));
}

void Chunk::ResetBuildSteps()
{
    m_BuildSteps.clear();
    m_BuildSteps.emplace_back(AppendVisibleFaces);
}

uint32_t Chunk::PackVertex(const uint32_t x, const uint32_t y, const uint32_t z, const uint32_t face, const uint32_t corner, const Block block)
{
    return (x & 0xFu)
        | ((z & 0xFu) << 4u)
        | ((y & 0xFFu) << 8u)
        | ((face & 0x7u) << 16u)
        | ((corner & 0x7u) << 19u)
        | ((static_cast<uint32_t>(block) & 0xFFu) << 22u);
}
