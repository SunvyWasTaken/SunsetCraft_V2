//
// Created by sunvy on 17/06/2026.
//

#include "TextureRegistry.h"

#include <stb_image.h>

#include "Render/Texture.h"
#include "Render/Image.h"

namespace
{
    constexpr int TextureSize = 32;
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

    void OpenFolder(const std::filesystem::path& path, std::vector<std::string>& textures)
    {
        textures.clear();
        for (const auto& entry : std::filesystem::directory_iterator(path))
        {
            if (!entry.is_regular_file())
                continue;

            if (!IsLoadableImage(entry.path()))
                continue;

            textures.emplace_back(entry.path());
        }
    }

    [[nodiscard]]
    glm::ivec2 CalculateAtlasSize(size_t textureSize, const size_t nbrTextures)
    {
        const size_t columns = AtlasTileCount;
        const size_t rows = std::ceil(static_cast<float>(nbrTextures) / columns);
        return {columns * textureSize, rows * textureSize};
    }

    void LoadTextures(const std::filesystem::path& path, std::vector<Sunset::Image>& images, glm::ivec2& atlasSize)
    {
        std::vector<std::string> textures;

        OpenFolder(path, textures);

        atlasSize = CalculateAtlasSize(32, textures.size());
    }
}

void TextureBlockRegistry::Init()
{
    LastTexturePos = glm::ivec2(0, 0);
    NumLastAddTexture = 0;
    m_UvList.clear();
    m_TextureIndexes.clear();

    const glm::ivec2 AtlasSize = CalculateAtlasSize(TextureSize, AtlasTileCount * AtlasTileCount);
    m_Texture = std::make_shared<Sunset::Textures>("BlockTextures", AtlasSize.x, AtlasSize.y);
}

void TextureBlockRegistry::Destroy()
{
    m_Texture.reset();
}

bool TextureBlockRegistry::LoadTexture(BlockId block, uint8_t side, const std::string& name)
{
    if (side >= 6)
        return false;

    // Apply the same texture if it was aleardy load.
    uint8_t texUV = 0;
    const auto textureIt = m_TextureIndexes.find(name);
    if (textureIt != m_TextureIndexes.end())
    {
        texUV = textureIt->second;
    }
    else
    {
        // Load the texture if it was never load
        const std::string TexName = RESOURCES + name;
        if (!IsLoadableImage(TexName))
        {
            LOG("SunsetCraft", warn, "This texture {} is not in a valid format to be load", name);
            return false;
        }
        texUV = NumLastAddTexture++;
        Sunset::Image img{TexName};
        m_Texture->Use();
        m_Texture->AddImageAt(img, LastTexturePos * TextureSize);
        m_TextureIndexes.emplace(name, texUV);

        ++LastTexturePos.x;
        if (LastTexturePos.x >= AtlasTileCount)
        {
            LastTexturePos.x = 0;
            ++LastTexturePos.y;
        }
    }

    m_UvList[block][side] = {texUV, name};
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
