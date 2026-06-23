//
// Created by sunvy on 23/06/2026.
//

#include "Inventaire.h"

#include "Registry/ItemRegistry.h"

Inventaire::Inventaire()
    : m_Slots()
{
}

Inventaire::~Inventaire()
{
}

bool Inventaire::Add(Item::Id id, uint16_t& amount)
{
    const auto& def = ItemRegistry::Get(id);
    for (auto& slot : m_Slots)
    {
        if (slot.id == id && slot.count < def.maxStack)
        {
            uint16_t space = def.maxStack - slot.count;
            uint16_t toAdd = std::min(space, amount);
            slot.count += toAdd;
            amount -= toAdd;
            if (amount <= 0)
                return true;
        }
    }

    for (auto& slot : m_Slots)
    {
        if (slot.Empty())
        {
            uint16_t toAdd = std::min(def.maxStack, amount);
            slot.id = id;
            slot.count = toAdd;
            amount -= toAdd;
            if (amount <= 0)
                return true;
        }
    }
    return false;
}

ItemStack & Inventaire::getSlot(const size_t index)
{
    return m_Slots[index];
}
