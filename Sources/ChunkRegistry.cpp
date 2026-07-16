//
// Created by sunvy on 14/06/2026.
//

#include "ChunkRegistry.h"

#include <queue>
#include <unordered_set>

#include "Chunk.h"
#include "RaycastHit.h"
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

    std::string fName;

    void LoadChunk(const glm::ivec2& position, const std::uint8_t renderDistance, ChunkRegistry* registry)
    {
        SS_PROFILE_FUNCTION();
        for (int32_t x = position.x - renderDistance; x <= position.x + renderDistance; ++x)
        {
            for (int32_t y = position.y - renderDistance; y <= position.y + renderDistance; ++y)
            {
                const glm::ivec2 key{x, y};
                const glm::ivec2 delta = key - position;
                const int dist2 = delta.x * delta.x + delta.y * delta.y;

                if (chunks.contains(key))
                    continue;

                const std::string fileName = std::format("{}_{}", x, y);
                if (ChunkSave cs; Sunset::SaveSystem::Load(SAVE_PATH + fName + "/" + fileName + ".bin", cs))
                {
                    chunks.try_emplace(cs.position, cs.position);
                    Chunk& chunk = chunks.at(cs.position);
                    chunk.m_Blocks = cs.blocks;
                    chunk.bIsDirty = true;
                    chunk.m_Registry = registry;
                    continue;
                }

                if (dist2 <= renderDistance * renderDistance)
                {
                    RequestChunkGeneration(key);
                }
            }
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

#pragma region LineTrace
    // void LineTrace(RaycastHit& hit, const glm::vec3& start, const glm::vec3& forward, float distance)
    // {
    //     hit.Clear();
    //
    //     // Direction du rayon
    //     glm::vec3 dir = glm::normalize(forward);
    //
    //     // Position voxel courante
    //     glm::ivec3 voxelPos = glm::floor(start);
    //
    //     // Sens de progression
    //     glm::ivec3 step;
    //     step.x = (dir.x > 0) ? 1 : (dir.x < 0 ? -1 : 0);
    //     step.y = (dir.y > 0) ? 1 : (dir.y < 0 ? -1 : 0);
    //     step.z = (dir.z > 0) ? 1 : (dir.z < 0 ? -1 : 0);
    //
    //     // Distance en t pour traverser un voxel
    //     glm::vec3 tDelta;
    //     tDelta.x = (dir.x != 0.0f) ? std::abs(1.0f / dir.x) : FLT_MAX;
    //     tDelta.y = (dir.y != 0.0f) ? std::abs(1.0f / dir.y) : FLT_MAX;
    //     tDelta.z = (dir.z != 0.0f) ? std::abs(1.0f / dir.z) : FLT_MAX;
    //
    //     // Distance jusqu'à la première frontière
    //     glm::vec3 tMax;
    //     tMax.x = (dir.x > 0)
    //         ? (voxelPos.x + 1 - start.x) * tDelta.x
    //         : (start.x - voxelPos.x) * tDelta.x;
    //
    //     tMax.y = (dir.y > 0)
    //         ? (voxelPos.y + 1 - start.y) * tDelta.y
    //         : (start.y - voxelPos.y) * tDelta.y;
    //
    //     tMax.z = (dir.z > 0)
    //         ? (voxelPos.z + 1 - start.z) * tDelta.z
    //         : (start.z - voxelPos.z) * tDelta.z;
    //
    //     float t = 0.0f;
    //     glm::ivec3 hitNormal(0);
    //
    //     // Boucle DDA
    //     while (t <= distance)
    //     {
    //         // Récupération du chunk / bloc
    //         BlockId blockId = ChunkRegistry::GetBlock(voxelPos);
    //         if (blockId != BlockRegistry::AIR)
    //         {
    //             hit.Hit = true;
    //             hit.blockPose = voxelPos;
    //             hit.BlockType = blockId;
    //             hit.hitNormal = hitNormal;
    //             return;
    //         }
    //
    //         // Avancer vers la frontière la plus proche
    //         if (tMax.x < tMax.y)
    //         {
    //             if (tMax.x < tMax.z)
    //             {
    //                 voxelPos.x += step.x;
    //                 t = tMax.x;
    //                 tMax.x += tDelta.x;
    //                 hitNormal = glm::ivec3(-step.x, 0, 0);
    //             }
    //             else
    //             {
    //                 voxelPos.z += step.z;
    //                 t = tMax.z;
    //                 tMax.z += tDelta.z;
    //                 hitNormal = glm::ivec3(0, 0, -step.z);
    //             }
    //         }
    //         else
    //         {
    //             if (tMax.y < tMax.z)
    //             {
    //                 voxelPos.y += step.y;
    //                 t = tMax.y;
    //                 tMax.y += tDelta.y;
    //                 hitNormal = glm::ivec3(0, -step.y, 0);
    //             }
    //             else
    //             {
    //                 voxelPos.z += step.z;
    //                 t = tMax.z;
    //                 tMax.z += tDelta.z;
    //                 hitNormal = glm::ivec3(0, 0, -step.z);
    //             }
    //         }
    //     }
    // }
#pragma endregion
}

ChunkRegistry::ChunkRegistry(int seed, const std::string &folderName, uint8_t renderDistance)
{
    INITLOG("ChunkRegistry");
    LOG("ChunkRegistry", info, "ChunkRegistry init");
    fName = folderName;
    m_RenderDistance = renderDistance;
    WorldGen::Init(seed);

    const uint32_t threadCount = std::max(1u, std::thread::hardware_concurrency() - 1);

    workers.reserve(threadCount);

    for (uint32_t i = 0; i < threadCount; ++i)
    {
        workers.emplace_back(WorkerLoop);
    }
}

Sunset::ReflectionType ChunkRegistry::Properties()
{
    Sunset::ReflectionType type;
    type.Name = "Chunk";
    type.Field("Render Distance", &ChunkRegistry::m_RenderDistance);
    return type;
}

void ChunkRegistry::OnUpdate(float dt)
{
    ScriptEntity::OnUpdate(dt);

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
    ScriptEntity::OnDraw();

    // todo : this is a temporary solution.
    auto* cam = GetComponent<Sunset::CameraComponent>();
    glm::vec3 loc = GetComponent<Sunset::TransformComponent>()->GetLocation();
    cam->camera.SetPosition(loc);
    cam->camera.SetForward(GetComponent<Sunset::TransformComponent>()->GetForwardVector());

    DrawChunk(cam->camera);
}

void ChunkRegistry::OnEndPlay()
{
    ScriptEntity::OnEndPlay();
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

    chunkToSave.emplace_back(it->second.m_Position, it->second.m_Blocks);
    return true;
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
