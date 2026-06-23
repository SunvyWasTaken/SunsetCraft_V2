//
// Created by sunvy on 23/06/2026.
//

#pragma once

#include "../Items/Item.h"
#include "../BlockRegistry.h"

struct ItemRegistry
{
    static void Init();
    static void Destroy();
    static ItemDef& Get(const std::string& name);
    static ItemDef& Get(Item::Id id);
};