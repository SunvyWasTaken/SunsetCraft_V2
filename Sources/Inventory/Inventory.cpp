//
// Created by sunvy on 23/06/2026.
//

#include "Inventory.h"

#include "BlockIconRender.h"
#include "GridPanel.h"
#include "HorizontalBox.h"
#include "Image.h"
#include "Core/Application.h"
#include "Core/ApplicationSetting.h"
#include "Overlay.h"
#include "Registry/ItemRegistry.h"
#include "Render/Texture.h"

Inventory::Inventory()
    : m_Overlay(nullptr)
    , m_Toolbar(nullptr)
    , m_Indicator(nullptr)
    , ShowInventory(false)
    , InventoryTexture(nullptr)
    , HotBarTexture(nullptr)
    , IndicatorTexture(nullptr)
    , m_Slots()
{
    glm::ivec2 winSize = Sunset::Application::GetSetting().WindowSize;

    InventoryTexture = std::make_unique<Sunset::Texture>();
    InventoryTexture->LoadImage(RESOURCES "Textures/gui/Inventory.png");

    constexpr glm::ivec2 inventorySize{650, 488};
    constexpr glm::ivec2 halfInvSize = inventorySize / 2;

    std::shared_ptr<SRmGUI::GridPanel> inv = nullptr;
    SRmGUI::SNewAssign<SRmGUI::Overlay>(m_Overlay)
        .Position({winSize.x/2 - halfInvSize.x, winSize.y/2 - halfInvSize.y})
        .Size(inventorySize)
        .Visibility(ShowInventory)
        .Child(
            SRmGUI::SNew<SRmGUI::Image>()
                .Image(InventoryTexture->GetId())
        ).Child(
            SRmGUI::SNewAssign<SRmGUI::GridPanel>(inv)
                .Column(9)
                .Row(4)
        );

    for (int i = 0; i < SlotCount; ++i)
    {
        inv->AddChild(m_Slots[i].GetDraw());
    }

    HotBarTexture = std::make_unique<Sunset::Texture>();
    HotBarTexture->LoadImage(RESOURCES "Textures/gui/hotbar.png");
    IndicatorTexture = std::make_unique<Sunset::Texture>();
    IndicatorTexture->LoadImage(RESOURCES "Textures/gui/hotbar_selection.png");

    const glm::ivec2 WinSize = Sunset::Application::GetSetting().WindowSize;

    std::shared_ptr<SRmGUI::HorizontalBox> HotBar = nullptr;
    SRmGUI::SNewAssign<SRmGUI::Overlay>(m_Toolbar)
        .Position({(WinSize.x/2)-364, WinSize.y-88})
        .Size({728, 88})
        .Child(
            SRmGUI::SNew<SRmGUI::Image>()
            .Image(HotBarTexture->GetId())
        )
        .Child(
            SRmGUI::SNewAssign<SRmGUI::HorizontalBox>(HotBar)
                .Padding({5.f, 5.f, 5.f, 5.f})
        ).Child(
            SRmGUI::SNewAssign<SRmGUI::Image>(m_Indicator)
            .Image(IndicatorTexture->GetId())
            .Size({88, 88})
            .Padding({-5.f, -5.f, -5.f, -5.f})
        );

    for (int i = 0; i < 9; ++i)
    {
        HotBar->AddChild(m_Slots[i].GetDraw());
    }
}

Inventory::~Inventory()
{
}

void Inventory::Update(float dt)
{
    for (auto& slot : m_Slots)
    {
        slot.Update(dt);
    }
}

bool Inventory::Add(Item::Id id, uint16_t& amount)
{
    const auto& def = ItemRegistry::Get(id);
    for (auto& slot : m_Slots)
    {
        if (slot.Id() == id && slot.Count() < def.maxStack)
        {
            uint16_t space = def.maxStack - slot.Count();
            uint16_t toAdd = std::min(space, amount);
            slot.Count() += toAdd;
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
            slot.Id() = id;
            slot.Count() = toAdd;
            amount -= toAdd;
            if (amount <= 0)
                return true;
        }
    }
    return false;
}

void Inventory::SetSelectedSlot(int nbr)
{
    m_Indicator->SetOffset({80.f * nbr, 0.f});
}

void Inventory::SetShowInventory(bool show)
{
    ShowInventory = show;
    m_Overlay->SetVisibility(ShowInventory);
}

void Inventory::ToggleShowInventory()
{
    ShowInventory = !ShowInventory;
    SetShowInventory(ShowInventory);
}

ItemStack & Inventory::getSlot(const size_t index)
{
    return (m_Slots[index])();
}
