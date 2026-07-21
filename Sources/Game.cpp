//
// Created by sunvy on 22/05/2026.
//

#include "Game.h"

#include <random>
#include <glm/gtc/type_ptr.hpp>

#include "ChunkRegistry.h"
#include "DayNightCycle.h"
#include "Image.h"
#include "Overlay.h"
#include "WorldParam.h"
#include "Core/Application.h"
#include "Core/WindowSetting.h"
#include "Core/GameInstance.h"
#include "GameFramework/Components/CameraComponent.h"
#include "GameFramework/Components/InputComponent.h"
#include "GameFramework/Components/NativeScriptComponent.h"
#include "GameFramework/Components/TransformComponent.h"
#include "GameFramework/World/Entity.h"
#include "Network/NetworkService.h"
#include "Player/PlayerScript.h"
#include "Registry/ItemRegistry.h"
#include "Registry/RegistryLoader.h"
#include "Render/Resources/Texture.h"
#include "SaveSystem/SaveSystem.h"

namespace
{
    WorldParam m_Param;

    Sunset::Entity player;

    int currentSelectItem = 0;
    int prevSelectItem = 1;

    // std::unique_ptr<Sunset::Drawable> BlockHandDrawable = nullptr;

    std::unique_ptr<Sunset::Texture> crosshairTex = nullptr;
}

GameLayer::GameLayer(WorldParam param)
    : Layer()
{
    m_Param = param;

    Sunset::SaveSystem::CreateFolder(SAVE_PATH + param.Name);

    RegistryLoader::Init();
}

GameLayer::~GameLayer()
{
    glm::vec3 playerLastPos = player.GetComponent<Sunset::TransformComponent>()->GetLocation();
    Sunset::SaveSystem::Save(SAVE_PATH + m_Param.Name + "/PlayerData.bin", playerLastPos);

    crosshairTex.reset();
    RegistryLoader::Destroy();
}

void GameLayer::Init()
{
    Layer::Init();

    GetWorld()->AddSystem<ChunkRegistrySystem>();

    player = GetGameInstance()->m_ActiveWorld->CreateEntity("Player");
    player.AddComponent<Sunset::TransformComponent>();
    if (glm::vec3 playerStartPosition; Sunset::SaveSystem::Load(SAVE_PATH + m_Param.Name + "/PlayerData.bin", playerStartPosition))
    {
        player.GetComponent<Sunset::TransformComponent>()->SetLocation(playerStartPosition);
    }
    player.AddComponent<Sunset::CameraComponent>().Activate(true);
    player.AddComponent<Sunset::InputComponent>();
    player.AddComponent<Sunset::NativeScriptComponent>().Bind<PlayerScript>();
    std::uint8_t renderDistance = 16;
#ifndef NDEBUG
    renderDistance = 2;
#endif
    player.AddComponent<ChunkRegistry>(m_Param.seed, m_Param.Name, renderDistance);

    constexpr glm::vec4 color{245.f/255.f, 71.f/255.f, 123.f/255.f, 1.f};

    std::uint16_t count = 64;
    auto& items = ItemRegistry::GetAll();
    for (const auto &id: items | std::views::keys)
    {
        std::uint16_t c = count;
        m_Inventory.Add(id, c);
    }

    std::shared_ptr<SRmGUI::Panel> panel = nullptr;
    SRmGUI::SNewAssign(panel).Fill();

    glm::ivec2 winSize = Sunset::Application::GetSetting().WindowSize;

    constexpr glm::ivec2 crossSize{25, 25};
    constexpr glm::ivec2 crossHalfSize = crossSize / 2;

    crosshairTex = std::make_unique<Sunset::Texture>();
    crosshairTex->LoadImage(RESOURCES "Textures/gui/crosshair.png");

    std::shared_ptr<SRmGUI::Image> cross = nullptr;
    SRmGUI::SNewAssign(cross)
        .Position({0, 0})
        .Anchors({0.5f, 0.5f}, {0.5f, 0.5f})
        .Size(crossSize)
        .Image(crosshairTex->GetId());

    panel->AddChild(cross);
    panel->AddChild(m_Inventory.m_Inventory);
    panel->AddChild(m_Inventory.m_Toolbar);

    AddToViewport(panel);
}

void GameLayer::OnUpdate(float dt)
{
    SS_PROFILE_FUNCTION();
    Layer::OnUpdate(dt);
    DayNightCycle::Update(dt);

    if (Sunset::NetworkService::IsInitialized())
        Sunset::NetworkService::Get().Update(dt);

    m_Inventory.Update(dt);

    glm::vec3 loc = player.GetComponent<Sunset::TransformComponent>()->GetLocation();

    PRINTSCREEN("Loc : {}", loc);

    if (prevSelectItem != currentSelectItem)
    {
        prevSelectItem = currentSelectItem;
    }
}

void GameLayer::OnDraw()
{
    SS_PROFILE_FUNCTION();
    DrawLightingEditor();
    Layer::OnDraw();
    m_Sky.Draw();
    auto* chunk = player.GetComponent<ChunkRegistry>();
    chunk->OnDraw();
}

bool GameLayer::OnEvent(const Sunset::Event::Type &event)
{
    SS_PROFILE_FUNCTION();
    Layer::OnEvent(event);

    GetGameInstance()->m_ActiveWorld->OnEvent(event);

    if (auto* mouseEvent = std::get_if<Sunset::Event::MouseScroll>(&event))
    {
        if (mouseEvent->offset.y != 0)
        {
            currentSelectItem -= mouseEvent->offset.y;
            if (currentSelectItem < 0)
                currentSelectItem = 8;
            else if (currentSelectItem >= 9)
                currentSelectItem = 0;

            m_Inventory.SetSelectedSlot(currentSelectItem);
            return true;
        }
    }
    return false;
}

void GameLayer::DrawLightingEditor()
{
    if (ImGui::GetCurrentContext() == nullptr)
        return;

    if (!ImGui::Begin("Lighting"))
    {
        ImGui::End();
        return;
    }

    LOG("SunsetCraft", info, "No light?")

    float timeOfDay = DayNightCycle::GetTimeOfDay();
    if (ImGui::SliderFloat("Time of day", &timeOfDay, 0.0f, 24.0f, "%.2f h"))
        DayNightCycle::SetTimeOfDay(timeOfDay);

    ImGui::Checkbox("Auto cycle", &DayNightCycle::AutoCycle());
    ImGui::DragFloat("Cycle speed", &DayNightCycle::CycleSpeed(), 0.01f, 0.0f, 6.0f, "%.2f h/s");

    const glm::vec3 sunDirection = DayNightCycle::GetSunDirection();
    ImGui::Text("Sun direction: %.2f %.2f %.2f", sunDirection.x, sunDirection.y, sunDirection.z);

    ImGui::End();
}
