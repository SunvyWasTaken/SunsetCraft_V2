//
// Created by sunvy on 23/06/2026.
//

#pragma once

#include "Registry/BlockRegistry.h"

namespace Item
{
    using Id = uint32_t;
    constexpr Id null = std::numeric_limits<uint32_t>::max();
}

enum class ItemType : uint8_t
{
    None,
    Block,
    Tool,
    MAX
};

struct ItemStack
{
    Item::Id id = Item::null;
    uint8_t count = 0;

    bool Empty() const;
};

struct ItemDef
{
    std::string name {};
    ItemType type = ItemType::None;

    uint16_t maxStack = 64;
    uint16_t iconIndex = 0;
    BlockId blockId;
};
