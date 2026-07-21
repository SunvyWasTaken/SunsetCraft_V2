//
// Created by sunvy on 14/06/2026.
//

#include "ChunkRegistry.h"

#include <algorithm>
#include <queue>
#include <unordered_set>
#include <vector>

#include "Chunk.h"
#include "RaycastHit.h"
#include "ShadowMap.h"
#include "GameFramework/Components/CameraComponent.h"
#include "GameFramework/Components/TransformComponent.h"
#include "Math/AABB.h"
#include "Render/Camera.h"
#include "SaveSystem/SaveSystem.h"
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

    std::unordered_map<glm::ivec2, Chunk, double_hash> chunks;

    struct ChunkSave
    {
        glm::ivec2 position;
        std::array<BlockId, SIZE_X * (SIZE_Y * 2) * SIZE_Z> blocks;
    };

    template <typename  Archive>
    void Serialize(Archive& ar, ChunkSave& save)
    {
        ar(save.position);
        ar(save.blocks);
    }

    std::vector<ChunkSave> chunkToSave;

    constexpr float TimeBtwSave = 10.f;
    float LastTimeSaved = 0;

#pragma region THREAD
    struct ChunkJob
    {
        glm::ivec2 position{};
        int distance2 = 0;
    };

    struct ChunkJobPriority
    {
        bool operator()(const ChunkJob& left, const ChunkJob& right) const
        {
            return left.distance2 > right.distance2;
        }
    };

    std::vector<std::jthread> workers;

    std::mutex jobMutex;
    std::condition_variable_any jobCv;
    std::priority_queue<ChunkJob, std::vector<ChunkJob>, ChunkJobPriority> jobs;

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

                job = jobs.top();
                jobs.pop();
            }

            GeneratedChunk generated{job.position};
            WorldGen::GenChunk(generated);

            if (stopToken.stop_requested())
                return;

            {
                std::lock_guard lock(resultMutex);
                results.push(std::move(generated));
            }
        }
    }

    void RequestChunkGeneration(const glm::ivec2& key, const int distance2)
    {
        if (chunks.contains(key))
            return;

        if (pendingChunks.contains(key))
            return;

        pendingChunks.insert(key);

        {
            std::lock_guard lock(jobMutex);
            jobs.push(ChunkJob{key, distance2});
        }

        jobCv.notify_one();
    }

    void MarkLoadedNeighborsDirty(const glm::ivec2& position)
    {
        for (const auto& dir : dirs)
        {
            const auto it = chunks.find(position + dir);
            if (it != chunks.end())
                it->second.bIsDirty = true;
        }
    }

    void MarkLoadedChunkDirty(const glm::ivec2& position)
    {
        const auto it = chunks.find(position);
        if (it != chunks.end())
            it->second.bIsDirty = true;
    }

    void MarkBoundaryNeighborsDirty(const glm::ivec2& chunkPosition, const glm::ivec3& localPosition)
    {
        if (localPosition.x == 0)
            MarkLoadedChunkDirty(chunkPosition + glm::ivec2{-1, 0});
        else if (localPosition.x == SIZE_X - 1)
            MarkLoadedChunkDirty(chunkPosition + glm::ivec2{1, 0});

        if (localPosition.z == 0)
            MarkLoadedChunkDirty(chunkPosition + glm::ivec2{0, -1});
        else if (localPosition.z == SIZE_Z - 1)
            MarkLoadedChunkDirty(chunkPosition + glm::ivec2{0, 1});
    }

    void ConsumeGeneratedChunks(ChunkRegistry* registry)
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
            chunk.m_Registry = registry;

            MarkLoadedNeighborsDirty(generated.position);
        }
    }
#pragma endregion // THREAD

    template <typename T>
    int WorldToChunk(T value, const int chunkSize)
    {
        return static_cast<int>(std::floor(value / chunkSize));
    }

    std::string fName;

    void LoadChunk(const glm::ivec2& position, const std::uint8_t renderDistance, ChunkRegistry* registry)
    {
        SS_PROFILE_FUNCTION();
        std::vector<ChunkJob> candidates;
        candidates.reserve((renderDistance * 2 + 1) * (renderDistance * 2 + 1));

        for (int32_t x = position.x - renderDistance; x <= position.x + renderDistance; ++x)
        {
            for (int32_t y = position.y - renderDistance; y <= position.y + renderDistance; ++y)
            {
                const glm::ivec2 key{x, y};
                const glm::ivec2 delta = key - position;
                const int dist2 = delta.x * delta.x + delta.y * delta.y;

                if (chunks.contains(key))
                    continue;

                if (dist2 > renderDistance * renderDistance)
                    continue;

                candidates.push_back({key, dist2});
            }
        }

        std::ranges::sort(candidates, [](const ChunkJob& left, const ChunkJob& right)
        {
            return left.distance2 < right.distance2;
        });

        for (const auto& candidate : candidates)
        {
            const auto& key = candidate.position;
            const std::string fileName = std::format("{}_{}", key.x, key.y);
            if (ChunkSave cs; Sunset::SaveSystem::Load(SAVE_PATH + fName + "/" + fileName + ".bin", cs))
            {
                chunks.try_emplace(cs.position, cs.position);
                Chunk& chunk = chunks.at(cs.position);
                chunk.m_Blocks = cs.blocks;
                chunk.bIsDirty = true;
                chunk.m_Registry = registry;
                MarkLoadedNeighborsDirty(cs.position);
                continue;
            }

            RequestChunkGeneration(key, candidate.distance2);
        }
    }

    void UnloadChunk(const glm::ivec2& position, const std::uint8_t renderDistance)
    {
        SS_PROFILE_FUNCTION();
        for (auto it = chunks.begin(); it != chunks.end(); )
        {
            int dx = std::abs(it->first.x - position.x);
            int dy = std::abs(it->first.y - position.y);

            if (dx > renderDistance || dy > renderDistance)
            {
                MarkLoadedNeighborsDirty(it->first);
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
        for (auto& chunk : chunks | std::views::values)
        {
            if (chunk.bIsDirty)
                chunk.BuildMesh();
        }
    }
}

ChunkRegistry::ChunkRegistry(int seed, const std::string &folderName, uint8_t renderDistance)
{
    INITLOG("ChunkRegistry");
    LOG("ChunkRegistry", info, "ChunkRegistry init");
    fName = folderName;
    m_RenderDistance = renderDistance;
    WorldGen::Init(seed);

    const uint32_t hardwareThreadCount = std::thread::hardware_concurrency();
    const uint32_t threadCount = std::max(1u, hardwareThreadCount > 1 ? hardwareThreadCount - 1 : 1u);

    workers.reserve(threadCount);

    for (uint32_t i = 0; i < threadCount; ++i)
    {
        workers.emplace_back(WorkerLoop);
    }
}

ChunkRegistry::~ChunkRegistry()
{
    OnEndPlay();
}

Sunset::ReflectionType ChunkRegistry::Properties()
{
    Sunset::ReflectionType type;
    type.Name = "Chunk";
    type.Field("Render Distance", &ChunkRegistry::m_RenderDistance);
    return type;
}

void ChunkRegistry::Update(float dt)
{
    static float waterTime = 0.0f;
    waterTime += dt;
    UpdateWaterTime(waterTime);

    if (const auto* tc = GetComponent<Sunset::TransformComponent>())
        UpdatePlayerPosition(tc->GetLocation());

    LastTimeSaved += dt;
    if (TimeBtwSave <= LastTimeSaved)
    {
        SaveChunk();
        LastTimeSaved = 0;
    }
}

void ChunkRegistry::OnDraw()
{
    OnDraw(ShadowRenderData{});
}

void ChunkRegistry::OnDraw(const ShadowRenderData& shadowData)
{
    // todo : this is a temporary solution.
    auto* cam = GetComponent<Sunset::CameraComponent>();
    glm::vec3 loc = GetComponent<Sunset::TransformComponent>()->GetLocation();
    cam->camera.SetPosition(loc);
    cam->camera.SetForward(GetComponent<Sunset::TransformComponent>()->GetForwardVector());

    DrawChunk(cam->camera, shadowData);
}

void ChunkRegistry::OnEndPlay()
{
    if (m_IsShutdown)
        return;

    m_IsShutdown = true;

    LOG("ChunkRegistry", info, "ChunkRegistry destroy");

    SaveChunk();

    for (auto& worker : workers)
    {
        worker.request_stop();
    }

    jobCv.notify_all();
    workers.clear();

    {
        std::lock_guard lock(jobMutex);
        std::priority_queue<ChunkJob, std::vector<ChunkJob>, ChunkJobPriority> empty;
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

    UnloadChunk(positionInChunk, m_RenderDistance);
    LoadChunk(positionInChunk, m_RenderDistance, this);
    ConsumeGeneratedChunks(this);
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

bool ChunkRegistry::SetBlock(const glm::vec3 &position, BlockId blockId)
{
    const glm::ivec2 positionInChunk{
        WorldToChunk(position.x, SIZE_X),
        WorldToChunk(position.z, SIZE_Z)};

    const auto it = chunks.find(positionInChunk);
    if (it == chunks.end())
        return false;

    if (!it->second.SetBlock(position, blockId))
        return false;

    const glm::ivec3 localPosition = glm::ivec3{position}
        - glm::ivec3{positionInChunk.x * SIZE_X, 0, positionInChunk.y * SIZE_Z};
    MarkBoundaryNeighborsDirty(positionInChunk, localPosition);

    chunkToSave.emplace_back(it->second.m_Position, it->second.m_Blocks);
    return true;
}

void ChunkRegistry::DrawChunk(const Sunset::Camera& camera)
{
    DrawChunk(camera, ShadowRenderData{});
}

void ChunkRegistry::DrawChunk(const Sunset::Camera& camera, const ShadowRenderData& shadowData)
{
    SS_PROFILE_FUNCTION();
    for (const auto &c: chunks | std::views::values)
    {
         if (camera.GetFrustum().IsVisible(Sunset::AABB{glm::vec3{c.m_Position.x * SIZE_X, -SIZE_Y, c.m_Position.y * SIZE_Z}, glm::vec3{c.m_Position.x * SIZE_X + SIZE_X, SIZE_Y, c.m_Position.y * SIZE_Z + SIZE_Z}}))
            c.Draw(shadowData);
    }
}

void ChunkRegistry::DrawShadowDepth(const Sunset::Shader& shadowShader)
{
    SS_PROFILE_FUNCTION();
    for (const auto& c : chunks | std::views::values)
    {
        c.DrawShadowDepth(shadowShader);
    }
}

void ChunkRegistry::SaveChunk()
{
    auto cs = std::move(chunkToSave);
    chunkToSave.clear();

    for (auto& c : cs)
    {
        std::string fileName = std::format("{}_{}", c.position.x, c.position.y);
        if (!Sunset::SaveSystem::Save(SAVE_PATH + fName + "/" + fileName + ".bin", c))
        {
            chunkToSave.emplace_back(c);
        }
    }
}

void ChunkRegistrySystem::Update(const float dt)
{
    IWorldSystem::Update(dt);
    m_World->Each<ChunkRegistry>([&](const Sunset::Entity& entity, ChunkRegistry& chunk_registry)
    {
        chunk_registry.Update(dt);
    });
}
