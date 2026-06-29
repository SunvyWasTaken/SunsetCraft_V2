//
// Created by sunvy on 23/06/2026.
//

#pragma once

#include "Items/Item.h"

class Inventaire
{
public:
    static constexpr int SlotCount = 36;

    Inventaire();
    virtual ~Inventaire();

    bool Add(Item::Id id, uint16_t& amount);
    ItemStack& getSlot(size_t index);

    void Draw() const;
private:
    std::array<ItemStack, SlotCount> m_Slots;
};
