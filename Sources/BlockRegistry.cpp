//
// Created by sunvy on 31/05/2026.
//

#include "BlockRegistry.h"

#include <unordered_set>

#include "TextureRegistry.h"
#include "Utility/UtilityFunction.h"

namespace
{
    bool bIsInitialized = false;

    std::unordered_map<std::string, BlockId> m_BlockRegistry;
    std::unordered_set<BlockId> m_TransparentBlocks;

    void FillRegistry(const nlohmann::json& blockJson)
    {
        if (blockJson.empty())
            return;

        BlockId currentBlockId = 0;

        for (auto& block_json : blockJson)
        {
            BlockType block;
            block.id = currentBlockId++;
            block.name = block_json["name"];

            if (block_json.contains("transparent"))
            {
                auto& IsTransparent = block_json["transparent"];
                if (IsTransparent.get<bool>())
                {
                    m_TransparentBlocks.emplace(block.id);
                }
            }

            if (block_json.contains("textures"))
            {
                auto& tex = block_json["textures"];
                if (tex.contains("all"))
                {
                    std::string all = tex.value("all", "");

                    for (uint8_t i = 0; i < 6; ++i)
                        TextureBlockRegistry::LoadTexture(block.id, i, all);
                }
                if (tex.contains("top"))
                    TextureBlockRegistry::LoadTexture(block.id, 3, tex["top"]);

                if (tex.contains("bottom"))
                    TextureBlockRegistry::LoadTexture(block.id, 2, tex["bottom"]);

                if (tex.contains("north"))
                    TextureBlockRegistry::LoadTexture(block.id, 4, tex["north"]);

                if (tex.contains("south"))
                    TextureBlockRegistry::LoadTexture(block.id, 5, tex["south"]);

                if (tex.contains("west"))
                    TextureBlockRegistry::LoadTexture(block.id, 1, tex["west"]);

                if (tex.contains("east"))
                    TextureBlockRegistry::LoadTexture(block.id, 0, tex["east"]);
            }

            m_BlockRegistry[block.name] = block.id;
        }

        bIsInitialized = true;
    }
}

void BlockRegistry::Init()
{
    if (bIsInitialized)
        return;

    nlohmann::json blockJson;
    if (!Sunset::UtilityFunction::LoadJson(SAVE_PATH "BlockReg.json", blockJson))
    {
        LOG("SunsetCraft", error, "BlockReg not Found");
        return;
    }

    FillRegistry(blockJson);

    AIR = Get("air");
    STONE = Get("stone");
    WATER = Get("water");
}

BlockId BlockRegistry::Get(const std::string &name)
{
    if (!bIsInitialized)
        LOG("BlockRegistry::Get", error, "Block not initialized");

    return m_BlockRegistry[name];
}

bool BlockRegistry::IsTransparent(BlockId id)
{
    return m_TransparentBlocks.contains(id);
}

BlockId BlockRegistry::AIR = 0;
BlockId BlockRegistry::WATER = 1;
BlockId BlockRegistry::STONE = 3;