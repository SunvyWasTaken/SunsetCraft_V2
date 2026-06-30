//
// Created by sunvy on 23/06/2026.
//

#pragma once

#include "ItemSlot.h"

namespace Sunset
{
    class Texture;
}

class Inventory
{
public:
    static constexpr int SlotCount = 36;

    Inventory();
    virtual ~Inventory();

    void Update(float dt);

    bool Add(Item::Id id, uint16_t& amount);

    void SetShowInventory(bool show);

    void ToggleShowInventory();

    ItemStack& getSlot(size_t index);

    std::shared_ptr<SRmGUI::Overlay> GetDraw();
private:
    bool ShowInventory;
    std::unique_ptr<Sunset::Texture> InventoryTexture;
    std::shared_ptr<SRmGUI::Overlay> m_Overlay;
    std::array<ItemSlot, SlotCount> m_Slots;
};
