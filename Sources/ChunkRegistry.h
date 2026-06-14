//
// Created by sunvy on 14/06/2026.
//

#pragma once

namespace Sunset
{
    class Camera;
}

struct ChunkRegistry
{
    static void Init(int seed, size_t renderDistance = 16);

    static void SetRenderDistance(size_t renderDistance);

    static void Destroy();

    static void UpdatePlayerPosition(const glm::vec3& position);

    static void GetBlock(const glm::vec3& position);

    static void DrawChunk(const Sunset::Camera& camera);
};
