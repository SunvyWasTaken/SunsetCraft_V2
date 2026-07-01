//
// Created by sunvy on 30/06/2026.
//

#pragma once

#include "Items/Item.h"

namespace SRmGUI
{
    class Text;
    class Image;
    class Overlay;
}

class ItemSlot
{
public:
    explicit ItemSlot(ItemStack* item = nullptr);
    ~ItemSlot();

    void Update(float dt);

    bool Empty() const
    {
        return items;
    }

    Item::Id& Id()
    {
        return items->id;
    }

    uint8_t& Count()
    {
        return items->count;
    }

    ItemStack* operator()()
    {
        return items;
    }

    std::shared_ptr<SRmGUI::Overlay> GetDraw();

private:
    ItemStack* items;
    std::shared_ptr<SRmGUI::Overlay> overlay;
    std::shared_ptr<SRmGUI::Image> m_Image;
    std::shared_ptr<SRmGUI::Text> m_Text;
};
