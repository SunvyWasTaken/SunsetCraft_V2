//
// Created by sunvy on 23/06/2026.
//

#include "Inventaire.h"

#include "Core/Application.h"
#include "Core/ApplicationSetting.h"
#include "Registry/ItemRegistry.h"

Inventaire::Inventaire()
    : m_Slots()
    , m_Drawable({1, 1}, {10, 10})
{
    glm::ivec2 winSize = Sunset::Application::GetSetting().WindowSize;
    m_Drawable.SetAnchor({winSize.x / 2.0f, winSize.y / 2.0f});
    m_Drawable.SetSize({500.0f, 500.0f});
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

void Inventaire::Draw() const
{
    m_Drawable.Draw();
}
