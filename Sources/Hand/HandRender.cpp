//
// Created by sunvy on 21/06/2026.
//

#include "HandRender.h"

#include "../Registry/BlockRegistry.h"
#include "../../SunsetEngine/Engine/Render/Resources/Drawable.h"

namespace
{
    BlockId m_HeldBlock = BlockRegistry::AIR;
    std::unique_ptr<Sunset::Drawable> m_BlockDrawable;
    std::unique_ptr<Sunset::Drawable> m_HandDrawable;
}

void HandRender::Init()
{
    m_BlockDrawable = std::make_unique<Sunset::Drawable>();
    m_HandDrawable = std::make_unique<Sunset::Drawable>();
}

void HandRender::Destroy()
{
    m_BlockDrawable.reset();
    m_HandDrawable.reset();
}

void HandRender::SetHeldBlock(BlockId block)
{
    m_HeldBlock = block;
}

void HandRender::Draw(const Sunset::Camera &camera)
{
}

void HandRender::Update(float dt)
{
}
