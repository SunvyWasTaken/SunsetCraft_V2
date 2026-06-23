//
// Created by sunvy on 23/06/2026.
//

#pragma once

#include "../Items/Item.h"

struct ItemRegistry
{
    static void Init();
    static void Destroy();
    static ItemDef& Get(const std::string& name);
    static ItemDef& Get(Item::Id id);
    static Item::Id GetId(const std::string& name);
};