//
// Created by sunvy on 17/06/2026.
//

#pragma once

#include "BlockRegistry.h"

namespace Sunset
{
    class Textures;
}

struct TextureBlockRegistry
{
    static void Init();

    static void Destroy();

    [[nodiscard]]
    static bool LoadTexture(Block block, const std::string& name);

    static uint8_t GetBlockId(Block block);

    [[nodiscard]]
    static std::shared_ptr<Sunset::Textures>& GetTexture();
};
