//
// Created by sunvy on 14/06/2026.
//

#include "ChunkRegistry.h"

#include <queue>
#include <unordered_set>

#include "Chunk.h"
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

    uint8_t m_RenderDistance = 0;
    std::unordered_map<glm::ivec2, Chunk, double_hash> chunks;

#pragma region THREAD
    struct ChunkJob
    {
        glm::ivec2 position{};
    };

    std::vector<std::jthread> workers;

    std::mutex jobMutex;
    std::condition_variable_any jobCv;
    std::queue<ChunkJob> jobs;

    std::mutex resultMutex;
    std::queue<GeneratedChunk> results;

    std::unordered_set<glm::ivec2, double_hash> pendingChunks;

    constexpr std::array<glm::ivec2, 4> dirs{glm::ivec2{-1,0}, {1,0}, {0,-1}, {0,1}};

    void WorkerLoop(std::stop_token stopToken)
    {
        while (!stopToken.stop_requested())
        {
            ChunkJob job;

            {
                std::unique_lock lock(jobMutex);

                jobCv.wait(lock, stopToken, []
                    {
                        return !jobs.empty();
                    });

                if (stopToken.stop_requested())
                    return;

                job = jobs.front();
                jobs.pop();
            }

            GeneratedChunk generated{job.position};
            WorldGen::GenChunk(generated);

            {
                std::lock_guard lock(resultMutex);
                results.push(std::move(generated));
            }
        }
    }

    void RequestChunkGeneration(const glm::ivec2& key)
    {
        if (chunks.contains(key))
            return;

        if (pendingChunks.contains(key))
            return;

        pendingChunks.insert(key);

        {
            std::lock_guard lock(jobMutex);
            jobs.push(ChunkJob{ key });
        }

        jobCv.notify_one();
    }

    void ConsumeGeneratedChunks()
    {
        std::queue<GeneratedChunk> localResults;

        {
            std::lock_guard lock(resultMutex);
            localResults.swap(results);
        }

        while (!localResults.empty())
        {
            GeneratedChunk generated = std::move(localResults.front());
            localResults.pop();

            pendingChunks.erase(generated.position);

            if (chunks.contains(generated.position))
                continue;

            auto& chunk = chunks.try_emplace(generated.position, generated.position).first->second;
            chunk.m_Blocks = std::move(generated.blocks);
            chunk.bIsDirty = true;

            for (auto& dir : dirs)
            {
                if (chunks.contains(generated.position + dir))
                    chunks.at(generated.position + dir).bIsDirty = true;
            }
        }
    }
#pragma endregion // THREAD

    template <typename T>
    int WorldToChunk(T value, const int chunkSize)
    {
        return static_cast<int>(std::floor(value / chunkSize));
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
                    RequestChunkGeneration(key);
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

    void BuildDirtyChunk()
    {
        SS_PROFILE_FUNCTION();
        for (auto& c : chunks | std::views::values)
        {
            if (c.bIsDirty)
                c.BuildMesh();
        }
    }
}

void ChunkRegistry::Init(const int seed, const uint8_t renderDistance)
{
    INITLOG("ChunkRegistry");
    LOG("ChunkRegistry", info, "ChunkRegistry init");
    m_RenderDistance = renderDistance;
    WorldGen::Init(seed);

    const uint32_t threadCount = std::max(1u, std::thread::hardware_concurrency() - 1);

    workers.reserve(threadCount);

    for (uint32_t i = 0; i < threadCount; ++i)
    {
        workers.emplace_back(WorkerLoop);
    }
}

void ChunkRegistry::Destroy()
{
    LOG("ChunkRegistry", info, "ChunkRegistry destroy");

    for (auto& worker : workers)
    {
        worker.request_stop();
    }

    jobCv.notify_all();
    workers.clear();

    {
        std::lock_guard lock(jobMutex);
        std::queue<ChunkJob> empty;
        jobs.swap(empty);
    }

    {
        std::lock_guard lock(resultMutex);
        std::queue<GeneratedChunk> empty;
        results.swap(empty);
    }

    pendingChunks.clear();

    chunks.clear();
    WorldGen::Destroy();
}

void ChunkRegistry::SetRenderDistance(const uint8_t renderDistance)
{
    m_RenderDistance = renderDistance;
}

void ChunkRegistry::UpdateWaterTime(const float time)
{
    Chunk::SetWaterTime(time);
}

void ChunkRegistry::UpdatePlayerPosition(const glm::vec3 &position)
{
    SS_PROFILE_FUNCTION();
    const glm::ivec2 positionInChunk{
        WorldToChunk(position.x, SIZE_X),
        WorldToChunk(position.z, SIZE_Z)};

    UnloadChunk(positionInChunk);
    LoadChunk(positionInChunk);
    ConsumeGeneratedChunks();
    BuildDirtyChunk();
}

BlockId ChunkRegistry::GetBlock(const glm::vec3 &position)
{
    const glm::ivec2 positionInChunk{
        WorldToChunk(position.x, SIZE_X),
        WorldToChunk(position.z, SIZE_Z)};

    const auto it = chunks.find(positionInChunk);
    if (it == chunks.end())
        return BlockRegistry::AIR;

    return it->second.GetBlock(position);
}

void ChunkRegistry::SetBlock(const glm::vec3 &position, BlockId blockId)
{
    const glm::ivec2 positionInChunk{
        WorldToChunk(position.x, SIZE_X),
        WorldToChunk(position.z, SIZE_Z)};

    const auto it = chunks.find(positionInChunk);
    if (it == chunks.end())
        return;

    it->second.SetBlock(position, blockId);
}

void ChunkRegistry::DrawChunk(const Sunset::Camera& camera)
{
    SS_PROFILE_FUNCTION();
    for (const auto &c: chunks | std::views::values)
    {
         if (camera.GetFrustum().IsVisible(Sunset::AABB{glm::vec3{c.m_Position.x * SIZE_X, -SIZE_Y, c.m_Position.y * SIZE_Z}, glm::vec3{c.m_Position.x * SIZE_X + SIZE_X, SIZE_Y, c.m_Position.y * SIZE_Z + SIZE_Z}}))
            c.Draw();
    }
}
