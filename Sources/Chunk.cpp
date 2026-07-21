//
// Created by sunvy on 31/05/2026.
//

#include "Chunk.h"

#include <glm/ext/matrix_transform.hpp>
#include <glad/glad.h>

#include "ChunkRegistry.h"
#include "DayNightCycle.h"
#include "ShadowMap.h"
#include "Registry/TextureRegistry.h"
#include "Render/Resources/Drawable.h"
#include "Render/Resources/Material.h"
#include "Render/Core/RenderCommand.h"
#include "Render/Core/Shader.h"
#include "Render/BufferObject/Buffers.h"
#include "Render/Meshes/Mesh.h"

namespace
{
    std::weak_ptr<Sunset::Shader> shader;
    std::weak_ptr<Sunset::Shader> TransparentShader;
    float WaterTime = 0.0f;

    struct FaceInstance
    {
        uint32_t data = 0;
        uint32_t ao = 0;
    };

    constexpr std::array<glm::ivec3, 6> checkDir = {
        glm::ivec3 {-1,  0,  0},
                { 1,  0,  0},
                { 0, -1,  0},
                { 0,  1,  0},
                { 0,  0, -1},
                { 0,  0,  1}
    };

    const std::array<glm::vec3, 36> faceVerts = {
        glm::vec3{0,0,0}, glm::vec3{0,1,0}, glm::vec3{0,1,1}, glm::vec3{0,1,1}, glm::vec3{0,0,1}, glm::vec3{0,0,0},
        glm::vec3{1,0,1}, glm::vec3{1,1,1}, glm::vec3{1,1,0}, glm::vec3{1,1,0}, glm::vec3{1,0,0}, glm::vec3{1,0,1},
        glm::vec3{0,0,0}, glm::vec3{0,0,1}, glm::vec3{1,0,1}, glm::vec3{1,0,1}, glm::vec3{1,0,0}, glm::vec3{0,0,0},
        glm::vec3{0,1,0}, glm::vec3{1,1,0}, glm::vec3{1,1,1}, glm::vec3{1,1,1}, glm::vec3{0,1,1}, glm::vec3{0,1,0},
        glm::vec3{1,0,0}, glm::vec3{1,1,0}, glm::vec3{0,1,0}, glm::vec3{0,1,0}, glm::vec3{0,0,0}, glm::vec3{1,0,0},
        glm::vec3{0,0,1}, glm::vec3{0,1,1}, glm::vec3{1,1,1}, glm::vec3{1,1,1}, glm::vec3{1,0,1}, glm::vec3{0,0,1}
    };

    bool IsInChunk(const int x, const int y, const int z)
    {
        return x >= 0 && x < SIZE_X && y >= -SIZE_Y && y < SIZE_Y && z >= 0 && z < SIZE_Z;
    }

    int GetIndex(const int x, const int y, const int z)
    {
        return x + z * SIZE_X + (y + SIZE_Y) * SIZE_X * SIZE_Z;
    }

    constexpr uint32_t WaterSurfaceBit = 1u << 29;

    uint32_t EncodePoint(const int x, const int y, const int z, const int dir, const int blockType, const int color, const bool waterSurface = false)
    {
        return   (static_cast<uint32_t>(x) & 0xFu)                  |
                ((static_cast<uint32_t>(y) & 0x1FFu) << 4)          |
                ((static_cast<uint32_t>(z) & 0xFu) << 13)           |
                ((static_cast<uint32_t>(dir) & 0x7u) << 17)         |
                ((static_cast<uint32_t>(blockType) & 0xFFu) << 20)  |
                ((static_cast<uint32_t>(color) & 0x1u) << 28)       |
                (waterSurface ? WaterSurfaceBit : 0u);
    }

    uint32_t PackAO(const std::array<uint8_t, 4>& values)
    {
        return (static_cast<uint32_t>(values[0]) & 0x3u)        |
              ((static_cast<uint32_t>(values[1]) & 0x3u) << 2)  |
              ((static_cast<uint32_t>(values[2]) & 0x3u) << 4)  |
              ((static_cast<uint32_t>(values[3]) & 0x3u) << 6);
    }

    glm::ivec3 WorldToChunk(const glm::ivec2& ChunkPos, const glm::ivec3& pos)
    {
        const glm::ivec3 localPos = pos - glm::ivec3{ChunkPos.x, 0, ChunkPos.y} * glm::ivec3{SIZE_X, 0, SIZE_Z};
        return localPos;
    }

    void UseBlockAtlas(Sunset::Drawable& drawable)
    {
        if (drawable.m_Material->m_Textures.empty())
            drawable.m_Material->m_Textures.emplace_back(TextureBlockRegistry::GetTexture());
    }

    void BindShadowMap(const ShadowRenderData& shadowData)
    {
        if (!shadowData.enabled || shadowData.depthTexture == 0)
            return;

        glActiveTexture(GL_TEXTURE0 + shadowData.textureUnit);
        glBindTexture(GL_TEXTURE_2D, shadowData.depthTexture);
        glActiveTexture(GL_TEXTURE0);
    }

    glm::ivec3 FaceAxisU(const int side)
    {
        if (side == 0 || side == 1)
            return {0, 1, 0};
        return {1, 0, 0};
    }

    glm::ivec3 FaceAxisV(const int side)
    {
        if (side == 4 || side == 5)
            return {0, 1, 0};
        return {0, 0, 1};
    }

    int AxisSign(const glm::vec3& vertex, const glm::ivec3& axis)
    {
        const float value = axis.x != 0 ? vertex.x : axis.y != 0 ? vertex.y : vertex.z;
        return value < 0.5f ? -1 : 1;
    }

    uint8_t VertexAO(const bool sideA, const bool sideB, const bool corner)
    {
        if (sideA && sideB)
            return 0;

        return static_cast<uint8_t>(3 - static_cast<int>(sideA) - static_cast<int>(sideB) - static_cast<int>(corner));
    }
}

Chunk::Chunk(const glm::vec2 &position)
    : m_Position(position)
    , m_Drawable(std::make_unique<Sunset::Drawable>())
    , m_TransparentDrawable(std::make_unique<Sunset::Drawable>())
    , m_Blocks()
{
    m_Blocks.fill(BlockRegistry::AIR);
}

Chunk::~Chunk()
{
}

void Chunk::Draw() const
{
    Draw(ShadowRenderData{});
}

void Chunk::Draw(const ShadowRenderData& shadowData) const
{
    glm::mat4 model = glm::translate(glm::mat4{1.0f}, {m_Position.x * SIZE_X, 0, m_Position.y * SIZE_Z});
    BindShadowMap(shadowData);

    if (m_Drawable->m_Material->m_Shader)
    {
        m_Drawable->m_Material->Set("u_TimeOfDay", DayNightCycle::GetTimeOfDay());
        m_Drawable->m_Material->Set("u_SunDirection", DayNightCycle::GetSunDirection());
        m_Drawable->m_Material->Set("u_LightSpaceMatrix", shadowData.lightSpaceMatrix);
        m_Drawable->m_Material->Set("u_ShadowMap", shadowData.textureUnit);
        m_Drawable->m_Material->Set("u_ShadowsEnabled", shadowData.enabled ? 1 : 0);
    }

    Sunset::RenderCommand::Submit(*m_Drawable, model);
    if (m_TransparentDrawable->m_Material->m_Shader)
    {
        m_TransparentDrawable->m_Material->Set("u_Time", WaterTime);
        m_TransparentDrawable->m_Material->Set("u_TimeOfDay", DayNightCycle::GetTimeOfDay());
        m_TransparentDrawable->m_Material->Set("u_SunDirection", DayNightCycle::GetSunDirection());
        m_TransparentDrawable->m_Material->Set("u_LightSpaceMatrix", shadowData.lightSpaceMatrix);
        m_TransparentDrawable->m_Material->Set("u_ShadowMap", shadowData.textureUnit);
        m_TransparentDrawable->m_Material->Set("u_ShadowsEnabled", shadowData.enabled ? 1 : 0);
    }
	Sunset::RenderCommand::Submit(*m_TransparentDrawable, model);
}

void Chunk::DrawShadowDepth(const Sunset::Shader& shadowShader) const
{
    if (!m_Drawable || !m_Drawable->m_Mesh)
        return;

    const glm::mat4 model = glm::translate(glm::mat4{1.0f}, {m_Position.x * SIZE_X, 0, m_Position.y * SIZE_Z});
    shadowShader.SetMat4("model", model);

    glBindVertexArray(m_Drawable->m_Mesh->GetVAO());
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, m_Drawable->m_Mesh->GetVertexCount());
}

void Chunk::SetWaterTime(const float time)
{
    WaterTime = time;
}

BlockId Chunk::GetBlock(const glm::vec3 &position) const
{
    const glm::ivec3 pos = WorldToChunk(m_Position, glm::ivec3{position});
    if (!IsInChunk(pos.x, pos.y, pos.z))
        return BlockRegistry::AIR;

    return m_Blocks[GetIndex(pos.x, pos.y, pos.z)];
}

bool Chunk::SetBlock(const glm::vec3 &position, BlockId block)
{
    const glm::ivec3 i = WorldToChunk(m_Position, position);
    if (!IsInChunk(i.x, i.y, i.z))
        return false;

    m_Blocks[GetIndex(i.x, i.y, i.z)] = block;
    bIsDirty = true;
    return true;
}

void Chunk::BuildMesh()
{
    SS_PROFILE_FUNCTION();
    std::vector<FaceInstance> points;
    std::vector<FaceInstance> TBlocks;

    const auto isOccludingBlock = [this](const int x, const int y, const int z)
    {
        if (!IsInChunk(x, y, z))
        {
            glm::ivec3 worldPos{x, y, z};
            worldPos += glm::ivec3{m_Position.x * SIZE_X, 0, m_Position.y * SIZE_Z};
            const BlockId block = m_Registry ? m_Registry->GetBlock(worldPos) : BlockRegistry::AIR;
            return block != BlockRegistry::AIR && !BlockRegistry::IsTransparent(block);
        }

        const BlockId block = m_Blocks[GetIndex(x, y, z)];
        return block != BlockRegistry::AIR && !BlockRegistry::IsTransparent(block);
    };

    const auto isWaterBlock = [this](const int x, const int y, const int z)
    {
        if (!IsInChunk(x, y, z))
        {
            glm::ivec3 worldPos{x, y, z};
            worldPos += glm::ivec3{m_Position.x * SIZE_X, 0, m_Position.y * SIZE_Z};
            return m_Registry && m_Registry->GetBlock(worldPos) == BlockRegistry::WATER;
        }

        return m_Blocks[GetIndex(x, y, z)] == BlockRegistry::WATER;
    };

    const auto computeFaceAO = [&](const int x, const int y, const int z, const int side)
    {
        constexpr std::array<int, 4> cornerVertexIndices{0, 1, 2, 4};
        std::array<uint8_t, 4> ao{};
        const glm::ivec3 normal = checkDir[side];
        const glm::ivec3 axisU = FaceAxisU(side);
        const glm::ivec3 axisV = FaceAxisV(side);

        const int offset = side * 6;
        for (size_t i = 0; i < cornerVertexIndices.size(); ++i)
        {
            const glm::vec3 vertex = faceVerts[offset + cornerVertexIndices[i]];
            const int uSign = AxisSign(vertex, axisU);
            const int vSign = AxisSign(vertex, axisV);

            const glm::ivec3 sideA = normal + axisU * uSign;
            const glm::ivec3 sideB = normal + axisV * vSign;
            const glm::ivec3 corner = normal + axisU * uSign + axisV * vSign;

            ao[i] = VertexAO(
                isOccludingBlock(x + sideA.x, y + sideA.y, z + sideA.z),
                isOccludingBlock(x + sideB.x, y + sideB.y, z + sideB.z),
                isOccludingBlock(x + corner.x, y + corner.y, z + corner.z));
        }

        return PackAO(ao);
    };

    const auto makeFace = [&](const int x, const int y, const int z, const int side, const int uv, const bool isGrass, const BlockId block)
    {
        const bool isWaterSurface = block == BlockRegistry::WATER && !isWaterBlock(x, y + 1, z);
        return FaceInstance{
            .data = EncodePoint(x, y + SIZE_Y, z, side, uv, isGrass, isWaterSurface),
            .ao = computeFaceAO(x, y, z, side)
        };
    };

    for (int x = 0; x < SIZE_X; ++x)
    {
        for (int z = 0; z < SIZE_Z; ++z)
        {
            for (int y = -SIZE_Y; y < SIZE_Y; ++y)
            {
                const int index = GetIndex(x, y, z);
                const BlockId b = m_Blocks[index];

                int side = -1;
                for (const auto& dir : checkDir)
                {
                    ++side;
                    if (!IsInChunk(x + dir.x, y + dir.y, z + dir.z))
                    {
                        glm::ivec3 worldPos{x + dir.x, y + dir.y, z + dir.z};
                        worldPos += glm::ivec3{m_Position.x * SIZE_X, 0, m_Position.y * SIZE_Z};
                        const BlockId testBlock = m_Registry ? m_Registry->GetBlock(worldPos) : BlockRegistry::AIR;
                        if (b == BlockRegistry::AIR)
                            continue;

                        bool IsGrass = (b == BlockRegistry::GRASS) && side == 3;

                        if (BlockRegistry::IsTransparent(b))
                        {
                            if (b != testBlock)
                                if (BlockRegistry::IsTransparent(testBlock))
                                    TBlocks.emplace_back(makeFace(x, y, z, side, TextureBlockRegistry::GetUvBlock(b, side), IsGrass, b));
                        }
                        else if (BlockRegistry::IsTransparent(testBlock))
                        {
                            points.emplace_back(makeFace(x, y, z, side, TextureBlockRegistry::GetUvBlock(b, side), IsGrass, b));
                        }
                    }
                    else if (const BlockId testBlock = m_Blocks[GetIndex(x + dir.x, y + dir.y, z + dir.z)]; BlockRegistry::IsTransparent(testBlock))
                    {
                        if (b == BlockRegistry::AIR)
                            continue;

                        bool IsGrass = (b == BlockRegistry::GRASS) && side == 3;

                        if (BlockRegistry::IsTransparent(b))
                        {
                            if (b != testBlock)
                                TBlocks.emplace_back(makeFace(x, y, z, side, TextureBlockRegistry::GetUvBlock(b, side), IsGrass, b));
                        }
                        else if (BlockRegistry::IsTransparent(testBlock))
                        {
                            points.emplace_back(makeFace(x, y, z, side, TextureBlockRegistry::GetUvBlock(b, side), IsGrass, b));
                        }
                    }
                }
            }
        }
    }

    {
        auto faceData = Sunset::BufferElement(Sunset::ShaderDataType::UInt, "data");
        auto aoData = Sunset::BufferElement(Sunset::ShaderDataType::UInt, "aoData");
        faceData.divisor = 1;
        aoData.divisor = 1;
        m_Drawable->m_Mesh = Sunset::Mesh::CreateVertexOnly(points.data(), sizeof(FaceInstance), points.size(), {faceData, aoData});
        m_Drawable->m_RenderState.DrawInstance = true;
        m_Drawable->m_RenderState.nbrInstance = 6;
        // m_Drawable->m_RenderState.wireframe = true;
        m_Drawable->m_RenderState.HasIndice = false;
        UseBlockAtlas(*m_Drawable);
        if (shader.expired())
        {
            m_Drawable->m_Material->m_Shader = std::make_shared<Sunset::Shader>(SHADERS_PATH "ChunkVertShader.vert", SHADERS_PATH "ChunkFragShader.frag");
            shader = m_Drawable->m_Material->m_Shader;
        }
        else
            m_Drawable->m_Material->m_Shader = shader.lock();
    }

    if (!TBlocks.empty())
    {
        auto faceData = Sunset::BufferElement(Sunset::ShaderDataType::UInt, "data");
        auto aoData = Sunset::BufferElement(Sunset::ShaderDataType::UInt, "aoData");
        faceData.divisor = 1;
        aoData.divisor = 1;
        m_TransparentDrawable->m_Mesh = Sunset::Mesh::CreateVertexOnly(TBlocks.data(), sizeof(FaceInstance), TBlocks.size(), {faceData, aoData});
        m_TransparentDrawable->m_RenderState.DrawInstance = true;
        m_TransparentDrawable->m_RenderState.nbrInstance = 6;
        m_TransparentDrawable->m_RenderState.HasIndice = false;
        m_TransparentDrawable->m_RenderState.blending = true;
        m_TransparentDrawable->m_RenderState.src = Sunset::BlendFactor::SrcAlpha;
        m_TransparentDrawable->m_RenderState.dest = Sunset::BlendFactor::OneMinusSrcAlpha;
        m_TransparentDrawable->m_RenderState.depthWrite = false;
        m_TransparentDrawable->m_RenderState.cullMode = Sunset::CullMode::None;
        UseBlockAtlas(*m_TransparentDrawable);
        if (TransparentShader.expired())
        {
            m_TransparentDrawable->m_Material->m_Shader = std::make_shared<Sunset::Shader>(SHADERS_PATH "WaterShader.vert", SHADERS_PATH "WaterShader.frag");
            TransparentShader = m_TransparentDrawable->m_Material->m_Shader;
        }
        else
            m_TransparentDrawable->m_Material->m_Shader = TransparentShader.lock();
    }

    bIsDirty = false;
}
