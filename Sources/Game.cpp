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
#include "Inventory/Inventory.h"
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

    // std::unique_ptr<Sunset::Drawable> BlockHandDrawable = nullptr;

    constexpr int ShadowResolution = 4096;

    std::unique_ptr<Sunset::Texture> crosshairTex = nullptr;
}

GameLayer::GameLayer(WorldParam param)
    : Layer()
    , m_ShadowPass({
        .size = ShadowResolution,
        .distance = 128.0f,
        .lightHeight = 180.0f,
        .vertexShaderPath = SHADERS_PATH "ShadowChunk.vert",
        .fragmentShaderPath = SHADERS_PATH "ShadowDepth.frag"
    })
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
    auto& inventory = player.AddComponent<Inventory>();
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
        inventory.Add(id, c);
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
        .Image(crosshairTex->GetRendererId().id);

    panel->AddChild(cross);
    panel->AddChild(inventory.m_Inventory);
    panel->AddChild(inventory.m_Toolbar);

    AddToViewport(panel);
}

void GameLayer::OnUpdate(float dt)
{
    SS_PROFILE_FUNCTION();
    Layer::OnUpdate(dt);
    DayNightCycle::Update(dt);

    if (Sunset::NetworkService::IsInitialized())
        Sunset::NetworkService::Get().Update(dt);

    if (auto* inventory = player.GetComponent<Inventory>())
        inventory->Update(dt);

    glm::vec3 loc = player.GetComponent<Sunset::TransformComponent>()->GetLocation();

    PRINTSCREEN("Loc : {}", loc);

}

void GameLayer::OnDraw()
{
    SS_PROFILE_FUNCTION();
    Layer::OnDraw();
    auto* chunk = player.GetComponent<ChunkRegistry>();
    const glm::vec3 playerLocation = player.GetComponent<Sunset::TransformComponent>()->GetLocation();

    m_ShadowPass.Render(playerLocation, DayNightCycle::GetSunDirection(), [chunk](const Sunset::Shader& shadowShader)
    {
        chunk->DrawShadowDepth(shadowShader);
    });
    const Sunset::ShadowRenderData shadowData = m_ShadowPass.GetRenderData();
    m_ShadowPass.BindForRead(shadowData.textureUnit);

    m_Sky.Draw();
    chunk->OnDraw(shadowData);
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
            if (auto* inventory = player.GetComponent<Inventory>())
            {
                inventory->SetSelectedSlot(inventory->GetSelectedSlot() - static_cast<int>(mouseEvent->offset.y));
                return true;
            }
        }
    }
    return false;
}
