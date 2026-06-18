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

    uint8_t NumLastAddTexture = 0;

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
    NumLastAddTexture = 0;
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
    // Apply the same texture if it was aleardy load.
    if (m_UvList.contains(block))
    {
        auto& obj = m_UvList[block];
        uint8_t texUV = 0;
        for (const auto&[UV, n] : obj)
        {
            if (n == name)
                texUV = UV;
        }
        obj[side].UV = texUV;
        return true;
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

    std::array<BlockTextureUV, 6> uvList;
    uvList[side] = BlockTextureUV{side, name};
    m_UvList.emplace(block, uvList);

    ++LastTexturePos.x;
    if (LastTexturePos.x >= AtlasTileCount)
    {
        LastTexturePos.x = 0;
        ++LastTexturePos.y;
    }
    return true;
}

uint8_t TextureBlockRegistry::GetUvBlock(BlockId block, uint8_t side)
{
    return m_UvList[block][side].UV;
}

std::shared_ptr<Sunset::Textures>& TextureBlockRegistry::GetTexture()
{
    return m_Texture;
}
