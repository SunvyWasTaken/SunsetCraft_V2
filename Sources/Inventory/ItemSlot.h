//
// Created by sunvy on 30/06/2026.
//

#pragma once

#include "Items/Item.h"

namespace SRmGUI
{
    class Overlay;
}

class ItemSlot
{
public:
    ItemSlot();
    ~ItemSlot();

    bool Empty() const
    {
        return items.Empty();
    }

    Item::Id& Id()
    {
        return items.id;
    }

    uint8_t& Count()
    {
        return items.count;
    }

    ItemStack& operator()()
    {
        return items;
    }
private:
    ItemStack items;
    std::shared_ptr<SRmGUI::Overlay> overlay;
};
