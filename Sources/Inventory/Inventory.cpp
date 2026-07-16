//
// Created by sunvy on 23/06/2026.
//

#include "Inventory.h"

#include "BlockIconRender.h"
#include "GridPanel.h"
#include "HorizontalBox.h"
#include "Image.h"
#include "Core/Application.h"
#include "Core/WindowSetting.h"
#include "Overlay.h"
#include "Registry/ItemRegistry.h"
#include "../../SunsetEngine/Engine/Render/Core/RenderCommand.h"
#include "../../SunsetEngine/Engine/Render/Resources/Texture.h"

Inventory::Inventory()
    : m_Inventory(nullptr)
    , m_Toolbar(nullptr)
    , m_Indicator(nullptr)
    , ShowInventory(false)
    , InventoryTexture(nullptr)
    , HotBarTexture(nullptr)
    , IndicatorTexture(nullptr)
{
    glm::ivec2 winSize = Sunset::Application::GetSetting().WindowSize;

    InventoryTexture = std::make_unique<Sunset::Texture>();
    InventoryTexture->LoadImage(RESOURCES "Textures/gui/Inventory.png");

    constexpr glm::ivec2 inventorySize{650, 488};
    constexpr glm::ivec2 halfInvSize = inventorySize / 2;

    std::shared_ptr<SRmGUI::HorizontalBox> barInv = nullptr;
    std::shared_ptr<SRmGUI::GridPanel> inv = nullptr;
    SRmGUI::SNewAssign<SRmGUI::Overlay>(m_Inventory)
    .Position({winSize.x/2 - halfInvSize.x, winSize.y/2 - halfInvSize.y})
    .Size(inventorySize)
    .Visibility(ShowInventory)
    .Child(
        SRmGUI::SNew<SRmGUI::Image>()
        .Image(InventoryTexture->GetId()))
    .Child(
        SRmGUI::SNewAssign<SRmGUI::GridPanel>(inv)
        .Padding({261.f, 83.f, 83.f, 128.f})
        .Column(9)
        .Row(3))
    .Child(
        SRmGUI::SNewAssign(barInv)
        .Padding({412, -89, -415, 82})
        .Size({inventorySize.x - 175, 45}));

    invSlots.reserve(SlotCount - 9);
    for (size_t i = 9; i < SlotCount; ++i)
    {
        invSlots.emplace_back(std::make_unique<ItemSlot>(&m_Slots, i));
        inv->AddChild(invSlots.back()->GetDraw());
    }
    crossbarSlots.reserve(9);
    for (size_t i = 0; i < 9; ++i)
    {
        crossbarSlots.emplace_back(std::make_unique<ItemSlot>(&m_Slots, i));
        barInv->AddChild(crossbarSlots.back()->GetDraw());
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

    crossbarSlots.reserve(9);
    for (size_t i = 0; i < 9; ++i)
    {
        crossbarSlots.emplace_back(std::make_unique<ItemSlot>(&m_Slots, i));
        HotBar->AddChild(crossbarSlots.back()->GetDraw());
    }
}

Inventory::~Inventory()
{
}

void Inventory::Update(float dt)
{
    if (ShowInventory)
        for (auto& slot : invSlots)
            slot->Update(dt);

    for (auto& slot : crossbarSlots)
        slot->Update(dt);
}

bool Inventory::Add(Item::Id id, uint16_t& amount)
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

void Inventory::SetSelectedSlot(int nbr)
{
    m_Indicator->SetOffset({80.f * nbr, 0.f});
}

void Inventory::SetShowInventory(bool show)
{
    ShowInventory = show;
    m_Inventory->SetVisibility(ShowInventory);
}

void Inventory::ToggleShowInventory()
{
    ShowInventory = !ShowInventory;
    Sunset::RenderCommand::ShowCursor(ShowInventory);
    SetShowInventory(ShowInventory);
}

ItemStack & Inventory::getSlot(const size_t index)
{
    return m_Slots[index];
}
