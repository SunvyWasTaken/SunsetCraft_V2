//
// Created by sunvy on 23/06/2026.
//

#pragma once

#include "GameFramework/Components/Component.h"
#include "ItemSlot.h"

namespace Sunset
{
    class Texture;
}

class Inventory : public Sunset::Component
{
public:

    Inventory();
    ~Inventory() override;

    void Update(float dt);

    bool Add(Item::Id id, uint16_t& amount);

    void SetSelectedSlot(int nbr);

    [[nodiscard]]
    int GetSelectedSlot() const;

    [[nodiscard]]
    bool GetSelectedBlock(BlockId& blockId) const;

    void ConsumeSelectedItem();

    void SetShowInventory(bool show);

    void ToggleShowInventory();

    ItemStack& getSlot(size_t index);

public:
    std::shared_ptr<SRmGUI::Overlay> m_Inventory;
    std::shared_ptr<SRmGUI::Overlay> m_Toolbar;
    std::shared_ptr<SRmGUI::Image> m_Indicator;
private:
    bool ShowInventory;
    int SelectedSlot;
    std::unique_ptr<Sunset::Texture> InventoryTexture;
    std::unique_ptr<Sunset::Texture> HotBarTexture;
    std::unique_ptr<Sunset::Texture> IndicatorTexture;
    std::vector<std::unique_ptr<ItemSlot>> invSlots;
    std::vector<std::unique_ptr<ItemSlot>> crossbarSlots;
    std::array<ItemStack, SlotCount> m_Slots;
};
