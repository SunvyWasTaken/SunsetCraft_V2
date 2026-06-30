//
// Created by sunvy on 23/06/2026.
//

#include "ItemRegistry.h"

#include <nlohmann/json.hpp>

#include "BlockRegistry.h"
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

        Item::Id currentItemId = Item::null;

        for (const auto& [name, blockJson] : items.items())
        {
            ItemDef item;
            item.name = name;
            // itemDef.type = blockJson["type"];
            if (blockJson.contains("stack"))
                item.maxStack = blockJson["stack"];
            else
                item.maxStack = 64;

            if (blockJson.contains("block"))
                item.blockId = BlockRegistry::Get(blockJson["block"]);

            itemDefs.emplace(++currentItemId, item);
            itemsName.emplace(item.name, currentItemId);
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

Item::Id ItemRegistry::GetId(const std::string &name)
{
    return itemsName[name];
}
