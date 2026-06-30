//
// Created by sunvy on 30/06/2026.
//

#include "BlockIconRender.h"

#include <memory>
#include <string>
#include <unordered_map>

#include "Registry/ItemRegistry.h"
#include "Registry/TextureRegistry.h"
#include "Render/Image.h"
#include "Log/Logger.h"

#include <glm/vec2.hpp>
#include "Render/Texture.h"

namespace
{
    constexpr int IconSize = 16;
    constexpr int AtlasTileCount = 16;

    std::shared_ptr<Sunset::Textures> TextureID = nullptr;
    std::unordered_map<Item::Id, glm::vec4> IconUvs;

    glm::vec4 BuildUv(const std::uint32_t index)
    {
        const float atlasSize = static_cast<float>(IconSize * AtlasTileCount);
        const float x = static_cast<float>((index % AtlasTileCount) * IconSize);
        const float y = static_cast<float>((index / AtlasTileCount) * IconSize);

        return {
            x / atlasSize,
            (y + IconSize) / atlasSize,
            (x + IconSize) / atlasSize,
            y / atlasSize
        };
    }

    std::string GetIconTextureName(const ItemDef& item)
    {
        // Use the top block texture as the inventory icon when available.
        std::string textureName = TextureBlockRegistry::GetTextureBlock(item.blockId, 3);
        if (textureName.empty())
            textureName = TextureBlockRegistry::GetTextureBlock(item.blockId, 0);

        return textureName;
    }
}

void BlockIconRender::Init()
{
    IconUvs.clear();

    TextureID = std::make_shared<Sunset::Textures>(
        "BlockIconAtlas",
        IconSize * AtlasTileCount,
        IconSize * AtlasTileCount
    );

    std::uint32_t iconIndex = 0;
    for (const auto& [id, item] : ItemRegistry::GetAll())
    {
        if (item.type != ItemType::Block)
            continue;

        if (iconIndex >= AtlasTileCount * AtlasTileCount)
        {
            LOG("SunsetCraft", error, "Block icon atlas is full, cannot load icon for {}", item.name);
            break;
        }

        const std::string textureName = GetIconTextureName(item);
        if (textureName.empty())
        {
            LOG("SunsetCraft", warn, "No block texture found for item icon {}", item.name);
            continue;
        }

        Sunset::Image image{RESOURCES + std::string("Textures/") + textureName};
        if (!image)
        {
            LOG("SunsetCraft", warn, "Invalid block icon texture {} for item {}", textureName, item.name);
            continue;
        }

        const glm::ivec2 atlasPosition{
            static_cast<int>(iconIndex % AtlasTileCount) * IconSize,
            static_cast<int>(iconIndex / AtlasTileCount) * IconSize
        };

        TextureID->AddImageAt(image, atlasPosition);
        IconUvs.emplace(id, BuildUv(iconIndex));
        ++iconIndex;
    }
}

void BlockIconRender::Destroy()
{
    IconUvs.clear();
    TextureID.reset();
}

std::uint32_t BlockIconRender::GetTexture()
{
    return TextureID ? TextureID->operator()() : 0;
}

glm::vec4 BlockIconRender::GetIconUv(Item::Id id)
{
    if (const auto it = IconUvs.find(id); it != IconUvs.end())
        return it->second;

    return {0.f, 1.f, 1.f, 0.f};
}
