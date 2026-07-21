//
// Created by sunvy on 14/06/2026.
//

#pragma once

#include "GameFramework/System/IWorldSystem.h"
#include "GameFramework/World/ScriptEntity.h"
#include "Registry/BlockRegistry.h"

namespace Sunset
{
    class Camera;
    class Shader;
}

struct ShadowRenderData;

class ChunkRegistry : public Sunset::Component
{
public:
    ChunkRegistry(int seed, const std::string& folderName, uint8_t renderDistance = 16);

    ~ChunkRegistry() override;

    Sunset::ReflectionType Properties() override;

    void Update(float dt);

    void OnDraw();
    void OnDraw(const ShadowRenderData& shadowData);

    void OnEndPlay();

    [[nodiscard]]
    BlockId GetBlock(const glm::vec3& position);

    bool SetBlock(const glm::vec3& position, BlockId blockId);

    void DrawChunk(const Sunset::Camera& camera);
    void DrawChunk(const Sunset::Camera& camera, const ShadowRenderData& shadowData);
    void DrawShadowDepth(const Sunset::Shader& shadowShader);

private:
    void UpdateWaterTime(float time);

    void UpdatePlayerPosition(const glm::vec3& position);

    void SaveChunk();

public:
    std::uint8_t m_RenderDistance = 16;

private:
    bool m_IsShutdown = false;
};

struct ChunkRegistrySystem : public Sunset::IWorldSystem
{
    using IWorldSystem::IWorldSystem;
    void Update(float dt);
};
