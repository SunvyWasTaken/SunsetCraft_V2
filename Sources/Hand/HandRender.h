//
// Created by sunvy on 21/06/2026.
//

#pragma once

namespace Sunset
{
    class Camera;
}

using BlockId = uint8_t;

struct HandRender
{
    static void Init();
    static void Destroy();

    static void SetHeldBlock(BlockId block);
    static void Update(float dt);
    static void Draw(const Sunset::Camera& camera);
};