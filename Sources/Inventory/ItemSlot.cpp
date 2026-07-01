//
// Created by sunvy on 30/06/2026.
//

#include "ItemSlot.h"

#include "BlockIconRender.h"
#include "Image.h"
#include "Overlay.h"
#include "Text.h"

ItemSlot::ItemSlot()
    : overlay(nullptr)
{
    SRmGUI::SNewAssign<SRmGUI::Overlay>(overlay)
        .Padding({5.f, 11.f, 9.f, 5.f})
        .Child(
            SRmGUI::SNewAssign<SRmGUI::Image>(m_Image)
            .Image(BlockIconRender::GetTexture())
            .Uv(BlockIconRender::GetIconUv(items.id))
        )
        .Child(
            SRmGUI::SNewAssign<SRmGUI::Text>(m_Text)
            .Text(std::format("{}", items.count))
            .Color({0.f, 0.f, 0.f, 1.f})
            .Padding({19.f, 0.f, 0.f, 29.f})
        );
}

ItemSlot::~ItemSlot()
{
}

void ItemSlot::Update(float dt)
{
    if (items.Empty())
    {
        overlay->SetVisibility(false);
        return;
    }

    overlay->SetVisibility(true);
    m_Image->SetImage(BlockIconRender::GetTexture());
    m_Image->SetUv(BlockIconRender::GetIconUv(items.id));
    m_Text->SetText(std::format("{}", items.count));
}

std::shared_ptr<SRmGUI::Overlay> ItemSlot::GetDraw()
{
    return overlay;
}
