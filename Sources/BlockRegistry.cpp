//
// Created by sunvy on 31/05/2026.
//

#include "BlockRegistry.h"

#include "Utility/UtilityFunction.h"

namespace
{
    std::unordered_map<Block, BlockType> m_BlockRegistry;
    std::unordered_map<std::string, BlockId> m_BlockRegistryName;

    bool bIsInitialized = false;

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

            if (block_json.contains("textures"))
            {
                auto& tex = block_json["textures"];
                if (tex.contains("all"))
                {
                    std::string all = tex.value("all", "");
                    for (auto& t : block.textures)
                        t = all;
                }
                if (tex.contains("top"))
                    block.textures[static_cast<int>(BlockFace::Top)] = tex["top"];

                if (tex.contains("bottom"))
                    block.textures[static_cast<int>(BlockFace::Bottom)] = tex["bottom"];

                if (tex.contains("north"))
                    block.textures[static_cast<int>(BlockFace::North)] = tex["north"];

                if (tex.contains("south"))
                    block.textures[static_cast<int>(BlockFace::South)] = tex["south"];

                if (tex.contains("west"))
                    block.textures[static_cast<int>(BlockFace::West)] = tex["west"];

                if (tex.contains("east"))
                    block.textures[static_cast<int>(BlockFace::East)] = tex["east"];
            }

            m_BlockRegistry[block.id] = block;
            m_BlockRegistryName[block.name] = block.id;
        }

        bIsInitialized = true;
    }
}

void BlockRegistry::Init()
{
    nlohmann::json blockJson;
    if (!Sunset::UtilityFunction::LoadJson(SAVE_PATH "BlockReg.json", blockJson))
    {
        LOG("SunsetCraft", error, "BlockReg not Found");
        return;
    }


}

Block BlockRegistry::Get(const std::string &name)
{
    return 0;
}

Block BlockRegistry::AIR = 0;
Block BlockRegistry::STONE = 1;
