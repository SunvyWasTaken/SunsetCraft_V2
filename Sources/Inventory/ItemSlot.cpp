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
        .Child(
            SRmGUI::SNew<SRmGUI::Image>()
            .Image(BlockIconRender::GetIconId(items.id))
        )
        .Child(
            SRmGUI::SNew<SRmGUI::Text>()
            .Text(std::format("{}", items.count))
            .Color({0.f, 0.f, 0.f, 1.f})
        );
}

ItemSlot::~ItemSlot()
{
}
