//
// Created by sunvy on 17/06/2026.
//

#include "TextureRegistry.h"

#include <stb_image.h>

#include "Render/Texture.h"
#include "Render/Image.h"

namespace
{
    constexpr int TextureSize = 16;
    constexpr int AtlasTileCount = 16;

    std::shared_ptr<Sunset::Textures> m_Texture;
    std::unordered_map<std::string, uint8_t> m_TextureIndexes;

    glm::ivec2 LastTexturePos = glm::ivec2(0, 0);

    struct BlockTextureUV
    {
        uint8_t UV = 0;
        std::string name;
    };

    std::unordered_map<BlockId, std::array<BlockTextureUV, 6>> m_UvList;

    bool IsLoadableImage(const std::filesystem::path& path)
    {
        int width = 0, height = 0, channels = 0;
        const std::string filepath = path.string();
        return stbi_info(filepath.c_str(), &width, &height, &channels);
    }
}

void TextureBlockRegistry::Init()
{
    LastTexturePos = glm::ivec2(0, 0);
    m_UvList.clear();
    m_TextureIndexes.clear();

    const glm::ivec2 AtlasSize{AtlasTileCount * TextureSize, AtlasTileCount * TextureSize};
    m_Texture = std::make_shared<Sunset::Textures>("BlockTextures", AtlasSize.x, AtlasSize.y);
}

void TextureBlockRegistry::Destroy()
{
    m_Texture.reset();
}

bool TextureBlockRegistry::LoadTexture(BlockId block, uint8_t side, const std::string& name)
{
    if (side >= 6)
    {
        LOG("SunsetCraft", warn, "Invalid side {} for block texture {}", side, name);
        return false;
    }


    uint8_t texUV = 0;
    if (const auto it = m_TextureIndexes.find(name); it != m_TextureIndexes.end())
    {
        texUV = it->second;
    }
    else
    {
        const uint16_t textureIndex = LastTexturePos.x + LastTexturePos.y * AtlasTileCount;
        if (textureIndex > std::numeric_limits<uint8_t>::max())
        {
            LOG("SunsetCraft", error, "Block texture atlas is full, cannot load {}", name);
            return false;
        }

        // Load the texture if it was never load
        const std::string TexName = RESOURCES + std::string("Textures/") + name;
        // if (!IsLoadableImage(TexName))
        // {
        //     LOG("SunsetCraft", warn, "This texture {} is not in a valid format to be load", name);
        //     return false;
        // }

        Sunset::Image img{TexName};
        m_Texture->AddImageAt(img, LastTexturePos * glm::ivec2{TextureSize, TextureSize});
        texUV = static_cast<uint8_t>(textureIndex);
        m_TextureIndexes.emplace(name, texUV);
        ++LastTexturePos.x;
        if (LastTexturePos.x >= AtlasTileCount)
        {
            LastTexturePos.x = 0;
            ++LastTexturePos.y;
        }
    }

    auto& uvList = m_UvList[block];
    uvList[side] = BlockTextureUV{texUV, name};
    return true;
}

uint8_t TextureBlockRegistry::GetUvBlock(BlockId block, uint8_t side)
{
    if (side >= 6)
    {
        LOG("SunsetCraft", warn, "Invalid side {} for block {}", side, block);
        return 0;
    }

    const auto blockIt = m_UvList.find(block);
    if (blockIt == m_UvList.end())
    {
        LOG("SunsetCraft", warn, "No texture registered for block {}", block);
        return 0;
    }

    return blockIt->second[side].UV;
}

std::string TextureBlockRegistry::GetTextureBlock(BlockId block, uint8_t side)
{
    if (side >= 6)
    {
        LOG("SunsetCraft", warn, "Invalid side {} for block {}", side, block);
        return "";
    }

    const auto blockIt = m_UvList.find(block);
    if (blockIt == m_UvList.end())
    {
        LOG("SunsetCraft", warn, "No texture registered for block {}", block);
        return "";
    }

    return blockIt->second[side].name;
}

std::shared_ptr<Sunset::Textures>& TextureBlockRegistry::GetTexture()
{
    return m_Texture;
}
