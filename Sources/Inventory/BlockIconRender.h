//
// Created by sunvy on 30/06/2026.
//

#pragma once

#include "Items/Item.h"

struct BlockIconRender
{
    static void Init();
    static void Destroy();
    static std::uint32_t GetTexture();
    static glm::vec4 GetIconUv(Item::Id id);
};
