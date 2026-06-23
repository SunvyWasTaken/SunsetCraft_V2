//
// Created by sunvy on 23/06/2026.
//

#include "ItemRegistry.h"

#include <nlohmann/json.hpp>

#include "Utility/UtilityFunction.h"

namespace
{
    std::unordered_map<std::string, Item::Id> itemsName;
    std::unordered_map<Item::Id, ItemDef> itemDefs;

    void LoadItems()
    {
        nlohmann::json file;
        if (!Sunset::UtilityFunction::LoadJson(SAVE_PATH "Item.json", file))
        {
            LOG("SunsetCraft", error, "Item.json not Found");
            return;
        }

        if (file.empty())
        {
            LOG("SunsetCraft", error, "Item.json is empty");
            return;
        }

        const auto& items = file["items"];

        Item::Id currentItemId = 0;

        for (const auto& [name, blockJson] : items.items())
        {
            ItemDef itemDef;
            itemDef.name = name;
            // itemDef.type = blockJson["type"];
            if (blockJson.contains("Stack"))
                itemDef.maxStack = blockJson["Stack"];
            else
                itemDef.maxStack = 64;

            itemDef.blockId = BlockRegistry::Get(blockJson["block"]);
        }
    }
}

void ItemRegistry::Init()
{
    LoadItems();
}

void ItemRegistry::Destroy()
{
    itemsName.clear();
    itemDefs.clear();
}

ItemDef & ItemRegistry::Get(const std::string &name)
{
    return Get(itemsName[name]);
}

ItemDef & ItemRegistry::Get(const Item::Id id)
{
    return itemDefs[id];
}
