//
// Created by sunvy on 14/06/2026.
//

#include "ChunkRegistry.h"

#include "Chunk.h"
#include "Noise.h"
#include "Math/AABB.h"
#include "Render/Camera.h"
#include "WorldGen/WorldGen.h"

namespace
{

    struct double_hash
    {
        template <typename T>
        std::size_t operator()(const T& t) const
        {
            auto h1 = std::hash<float>{}(t.x);
            auto h2 = std::hash<float>{}(t.y);
            return h1 ^ (h2 << 1);
        }
    };

    int32_t m_RenderDistance = 0;
    std::unordered_map<glm::ivec2, Chunk, double_hash> chunks;

    template <typename T>
    int WorldToChunk(T value)
    {
        return static_cast<int>(std::floor(value / SIZE_X));
    }

    void LoadChunk(const glm::ivec2& position)
    {
        SS_PROFILE_FUNCTION();
        for (int32_t x = position.x - m_RenderDistance; x <= position.x + m_RenderDistance; ++x)
        {
            for (int32_t y = position.y - m_RenderDistance; y <= position.y + m_RenderDistance; ++y)
            {
                const glm::ivec2 key{x, y};
                const glm::ivec2 delta = key - position;
                const int dist2 = delta.x * delta.x + delta.y * delta.y;

                if (chunks.contains(key))
                    continue;

                if (dist2 <= m_RenderDistance * m_RenderDistance)
                {
                    auto& c = (chunks.try_emplace(key, key).first)->second;
                    WorldGen::GenChunk(c);
                }
            }
        }
    }

    void UnloadChunk(const glm::ivec2& position)
    {
        SS_PROFILE_FUNCTION();
        for (auto it = chunks.begin(); it != chunks.end(); )
        {
            int dx = std::abs(it->first.x - position.x);
            int dy = std::abs(it->first.y - position.y);

            if (dx > m_RenderDistance || dy > m_RenderDistance)
            {
                it = chunks.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}

void ChunkRegistry::Init(int seed, const size_t renderDistance)
{
    INITLOG("ChunkRegistry");
    LOG("ChunkRegistry", info, "ChunkRegistry init");
    m_RenderDistance = renderDistance;
    WorldGen::Init(seed);
}

void ChunkRegistry::Destroy()
{
    LOG("ChunkRegistry", info, "ChunkRegistry destroy");
    chunks.clear();
    WorldGen::Destroy();
}

void ChunkRegistry::UpdatePlayerPosition(const glm::vec3 &position)
{
    SS_PROFILE_FUNCTION();
    const glm::ivec2 positionInChunk{
        WorldToChunk(position.x),
        WorldToChunk(position.z)};

    UnloadChunk(positionInChunk);
    LoadChunk(positionInChunk);
}

void ChunkRegistry::GetBlock(const glm::vec3 &position)
{
}

void ChunkRegistry::DrawChunk(const Sunset::Camera& camera)
{
    for (const auto &c: chunks | std::views::values)
    {
         if (camera.GetFrustum().IsVisible(Sunset::AABB{glm::vec3{c.m_Position.x * SIZE_X, -SIZE_Y, c.m_Position.y * SIZE_Z}, glm::vec3{c.m_Position.x * SIZE_X + SIZE_X, SIZE_Y, c.m_Position.y * SIZE_Z + SIZE_Z}}))
            c.Draw();
    }
}
