//
// Created by sunvy on 14/06/2026.
//

#pragma once

#include "BlockRegistry.h"

namespace Sunset
{
    class Camera;
}

struct ChunkRegistry
{
    static void Init(int seed, uint8_t renderDistance = 16);

    static void Destroy();

    static void SetRenderDistance(uint8_t renderDistance);

    static void UpdateWaterTime(float time);

    static void UpdatePlayerPosition(const glm::vec3& position);

    [[nodiscard]]
    static BlockId GetBlock(const glm::vec3& position);

    static void DrawChunk(const Sunset::Camera& camera);
};
