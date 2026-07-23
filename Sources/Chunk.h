//
// Created by sunvy on 31/05/2026.
//

#pragma once

#include "Registry/BlockRegistry.h"
#include "Render/Pipeline/ShadowPass.h"

#define SIZE_X 16
#define SIZE_Y 255
#define SIZE_Z 16

struct ChunkRegistry;

namespace Sunset
{
    class Drawable;
    class Shader;
}

class Chunk
{
public:
    explicit Chunk(const glm::vec2& position);

    ~Chunk();

    void Draw() const;
    void Draw(const Sunset::ShadowRenderData& shadowData) const;
    void DrawShadowDepth(const Sunset::Shader& shadowShader) const;

    BlockId GetBlock(const glm::vec3& position) const;

    bool SetBlock(const glm::vec3& position, BlockId block);

    static void SetWaterTime(float time);

    void BuildMesh();

    glm::ivec2 m_Position;
    std::unique_ptr<Sunset::Drawable> m_Drawable;
	std::unique_ptr<Sunset::Drawable> m_TransparentDrawable;
    bool bIsDirty = true;
    std::array<BlockId, SIZE_X * (SIZE_Y * 2) * SIZE_Z> m_Blocks;

    ChunkRegistry* m_Registry = nullptr;
};
