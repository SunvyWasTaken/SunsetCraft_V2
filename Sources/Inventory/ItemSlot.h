//
// Created by sunvy on 30/06/2026.
//

#pragma once

#include "Items/Item.h"

static constexpr int SlotCount = 36;

namespace SRmGUI
{
    class Text;
    class Image;
    class Overlay;
}

class ItemSlot
{
public:
    explicit ItemSlot(std::array<ItemStack, SlotCount>* slot, size_t index);
    ~ItemSlot();

    void Update(float dt);

    std::shared_ptr<SRmGUI::Overlay> GetDraw();

private:
    size_t itemSlot;
    std::array<ItemStack, SlotCount>* m_Slots;
    std::shared_ptr<SRmGUI::Overlay> overlay;
    std::shared_ptr<SRmGUI::Image> m_Image;
    std::shared_ptr<SRmGUI::Text> m_Text;
};
