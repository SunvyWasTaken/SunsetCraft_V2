//
// Created by sunvy on 30/06/2026.
//

#include "ItemSlot.h"

#include "BlockIconRender.h"
#include "Image.h"
#include "Overlay.h"
#include "Text.h"

ItemSlot::ItemSlot(std::array<ItemStack, SlotCount>* slot, const size_t index)
    : itemSlot(index)
    , m_Slots(slot)
    , overlay(nullptr)
{
    SRmGUI::SNewAssign<SRmGUI::Overlay>(overlay)
        .Padding({5.f, 11.f, 9.f, 5.f})
        .Child(
            SRmGUI::SNewAssign<SRmGUI::Image>(m_Image)
            .Image(BlockIconRender::GetTexture())
            .Uv(BlockIconRender::GetIconUv((*m_Slots)[itemSlot].id))
            .OnDragDetect([this](SRmGUI::DragDropPayload& payload)->bool
            {
                LOG("SunsetCraft", info, "On Drag detected!!!");
                payload.Type = "ItemSlot";
                payload.Data = this;
                return true;
            })
            .CanAcceptDrag([](const SRmGUI::DragDropPayload& payload)->bool
            {
                return payload.Type == "ItemSlot";
            })
            .OnDrop([this](const SRmGUI::DragDropPayload& payload)->bool
            {
                LOG("SunsetCraft", info, "On Drop detected!!!");
                if (auto* source = static_cast<ItemSlot*>(payload.Data))
                {
                    const size_t trans = source->itemSlot;
                    source->itemSlot = itemSlot;
                    itemSlot = trans;
                }
                return true;
            })
        )
        .Child(
            SRmGUI::SNewAssign<SRmGUI::Text>(m_Text)
            .Text(std::format("{}", (*m_Slots)[itemSlot].count))
            .Color({0.f, 0.f, 0.f, 1.f})
            .Offset({26.f, 18.f})
        );
}

ItemSlot::~ItemSlot()
{
}

void ItemSlot::Update(float dt)
{
    if ((*m_Slots)[itemSlot].Empty())
    {
        overlay->SetVisibility(false);
        return;
    }

    overlay->SetVisibility(true);
    m_Image->SetImage(BlockIconRender::GetTexture());
    m_Image->SetUv(BlockIconRender::GetIconUv((*m_Slots)[itemSlot].id));
    m_Text->SetText(std::format("{}", (*m_Slots)[itemSlot].count));
}

std::shared_ptr<SRmGUI::Overlay> ItemSlot::GetDraw()
{
    return overlay;
}
