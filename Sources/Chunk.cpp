//
// Created by sunvy on 31/05/2026.
//

#include "Chunk.h"

#include "ChunkRegistry.h"
#include "TextureRegistry.h"
#include "Render/Drawable.h"
#include "Render/Material.h"
#include "Render/RenderCommande.h"
#include "Render/Shader.h"
#include "Render/BufferObject/Buffers.h"
#include "Render/Meshes/Mesh.h"

namespace
{
    std::weak_ptr<Sunset::Shader> shader;
    std::weak_ptr<Sunset::Shader> TransparentShader;

    constexpr std::array<glm::ivec3, 6> checkDir = {
        glm::ivec3 {-1,  0,  0},
                { 1,  0,  0},
                { 0, -1,  0},
                { 0,  1,  0},
                { 0,  0, -1},
                { 0,  0,  1}
    };

    bool IsInChunk(const int x, const int y, const int z)
    {
        return x >= 0 && x < SIZE_X && y >= -SIZE_Y && y < SIZE_Y && z >= 0 && z < SIZE_Z;
    }

    int GetIndex(const int x, const int y, const int z)
    {
        return x + z * SIZE_X + (y + SIZE_Y) * SIZE_X * SIZE_Z;
    }

    uint32_t EncodePoint(const int x, const int y, const int z, const int dir, const int blockType)
    {
        return   (static_cast<uint32_t>(x) & 0xFu)                  |
                ((static_cast<uint32_t>(y) & 0x1FFu) << 4)          |
                ((static_cast<uint32_t>(z) & 0xFu) << 13)           |
                ((static_cast<uint32_t>(dir) & 0x7u) << 17)         |
                ((static_cast<uint32_t>(blockType) & 0xFFu) << 20);
    }

    glm::ivec3 WorldToChunk(const glm::ivec2& ChunkPos, const glm::ivec3& pos)
    {
        const glm::ivec3 localPos = pos - glm::ivec3{ChunkPos.x, 0, ChunkPos.y} * glm::ivec3{SIZE_X, 0, SIZE_Z};
        return localPos;
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
    Sunset::RenderCommande::Submit(*m_Drawable);
	Sunset::RenderCommande::Submit(*m_TransparentDrawable);
}

BlockId Chunk::GetBlock(const glm::vec3 &position) const
{
    const glm::ivec3 pos = WorldToChunk(m_Position, glm::ivec3{position});
    if (!IsInChunk(pos.x, pos.y, pos.z))
        return BlockRegistry::AIR;

    return m_Blocks[GetIndex(pos.x, pos.y, pos.z)];
}

void Chunk::BuildMesh()
{
    SS_PROFILE_FUNCTION();
    std::vector<uint32_t> points;
    std::vector<uint32_t> TBlocks;
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
                        const BlockId testBlock = ChunkRegistry::GetBlock(worldPos);
                        if (b == BlockRegistry::AIR)
                            continue;

                        if (BlockRegistry::IsTransparent(b))
                        {
                            if (b != testBlock)
                                TBlocks.emplace_back(EncodePoint(x, y + SIZE_Y, z, side, TextureBlockRegistry::GetUvBlock(b, side)));
                        }
                        else if (BlockRegistry::IsTransparent(testBlock))
                        {
                            points.emplace_back(EncodePoint(x, y + SIZE_Y, z, side, TextureBlockRegistry::GetUvBlock(b, side)));
                        }
                    }
                    else if (const BlockId testBlock = m_Blocks[GetIndex(x + dir.x, y + dir.y, z + dir.z)]; BlockRegistry::IsTransparent(testBlock))
                    {
                        if (b == BlockRegistry::AIR)
                            continue;

                        if (BlockRegistry::IsTransparent(b))
                        {
                            if (b != testBlock)
                                TBlocks.emplace_back(EncodePoint(x, y + SIZE_Y, z, side, TextureBlockRegistry::GetUvBlock(b, side)));
                        }
                        else if (BlockRegistry::IsTransparent(testBlock))
                        {
                            points.emplace_back(EncodePoint(x, y + SIZE_Y, z, side, TextureBlockRegistry::GetUvBlock(b, side)));
                        }
                    }
                }
            }
        }
    }

    {
        auto faceData = Sunset::BufferElement(Sunset::ShaderDataType::UInt, "data");
        faceData.divisor = 1;
        m_Drawable->m_Mesh = Sunset::Mesh::CreateVertexOnly(points.data(), sizeof(uint32_t), points.size(), {faceData});
        m_Drawable->m_Position = {m_Position.x * SIZE_X, 0, m_Position.y * SIZE_Z};
        m_Drawable->m_RenderState.DrawInstance = true;
        m_Drawable->m_RenderState.nbrInstance = 6;
        // m_Drawable->m_RenderState.wireframe = true;
        m_Drawable->m_RenderState.HasIndice = false;
        m_Drawable->m_Material->m_Textures.emplace_back(TextureBlockRegistry::GetTexture());
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
        faceData.divisor = 1;
        m_TransparentDrawable->m_Mesh = Sunset::Mesh::CreateVertexOnly(TBlocks.data(), sizeof(uint32_t), TBlocks.size(), {faceData});
        m_TransparentDrawable->m_Position = {m_Position.x * SIZE_X, 0, m_Position.y * SIZE_Z};
        m_TransparentDrawable->m_RenderState.DrawInstance = true;
        m_TransparentDrawable->m_RenderState.nbrInstance = 6;
        m_TransparentDrawable->m_RenderState.HasIndice = false;
        m_TransparentDrawable->m_RenderState.blending = true;
        // Todo check pourquoi est ce que lorsque la transparence est la le tous devient noir check dans l'engine.
        m_TransparentDrawable->m_RenderState.src = Sunset::BlendFactor::SrcAlpha;
        m_TransparentDrawable->m_RenderState.dest = Sunset::BlendFactor::OneMinusSrcAlpha;
        m_TransparentDrawable->m_RenderState.depthWrite = false;
        m_TransparentDrawable->m_RenderState.cullMode = Sunset::CullMode::None;
        m_TransparentDrawable->m_Material->m_Textures.emplace_back(TextureBlockRegistry::GetTexture());
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
