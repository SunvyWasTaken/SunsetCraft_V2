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

    static bool LoadTexture(BlockId block, uint8_t side, const std::string& name);

    static uint8_t GetUvBlock(BlockId block, uint8_t side);

    static std::string GetTextureBlock(BlockId block, uint8_t side);

    [[nodiscard]]
    static std::shared_ptr<Sunset::Textures>& GetTexture();
};
