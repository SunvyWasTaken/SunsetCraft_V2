//
// Created by sunvy on 30/06/2026.
//

#include "BlockIconRender.h"

#include "Render/Shader.h"

namespace
{
    std::shared_ptr<Sunset::Shader> blockIconShader = nullptr;
}

void BlockIconRender::Init()
{
    blockIconShader = std::make_shared<Sunset::Shader>(SHADERS_PATH "BlockIcon.vert", SHADERS_PATH "BlockIcon.frag");

}

void BlockIconRender::Destroy()
{
    blockIconShader.reset();
}

std::uint32_t BlockIconRender::GetIconId(Item::Id id)
{
    return 0;
}
