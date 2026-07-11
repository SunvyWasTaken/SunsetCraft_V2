//
// Created by sunvy on 22/05/2026.
//

#include "Game.h"

#include <imgui.h>
#include <random>
#include <glm/gtc/type_ptr.hpp>

#include "Registry/BlockRegistry.h"
#include "ChunkRegistry.h"
#include "Image.h"
#include "Overlay.h"
#include "RaycastHit.h"
#include "WorldParam.h"
#include "Core/Application.h"
#include "Core/ApplicationSetting.h"
#include "Core/GameInstance.h"
#include "GameFramework/Components/CameraComponent.h"
#include "GameFramework/Components/NativeScriptComponent.h"
#include "GameFramework/Components/TransformComponent.h"
#include "GameFramework/World/Entity.h"
#include "Network/NetworkService.h"
#include "Player/PlayerScript.h"
#include "Registry/ItemRegistry.h"
#include "Registry/RegistryLoader.h"
#include "Render/RenderCommande.h"
#include "Render/Texture.h"
#include "SaveSystem/SaveSystem.h"

namespace
{
    WorldParam m_Param;

    Sunset::Entity player;

    int currentSelectItem = 0;
    int prevSelectItem = 1;

    // std::unique_ptr<Sunset::Drawable> BlockHandDrawable = nullptr;

    std::unique_ptr<Sunset::Texture> crosshairTex = nullptr;

    constexpr float TimeBtwSave = 10.f;
    float LastTimeSaved = 0;

#pragma region LineTrace
    void LineTrace(RaycastHit& hit, const glm::vec3& start, const glm::vec3& forward, float distance)
    {
        hit.Clear();

        // Direction du rayon
        glm::vec3 dir = glm::normalize(forward);

        // Position voxel courante
        glm::ivec3 voxelPos = glm::floor(start);

        // Sens de progression
        glm::ivec3 step;
        step.x = (dir.x > 0) ? 1 : (dir.x < 0 ? -1 : 0);
        step.y = (dir.y > 0) ? 1 : (dir.y < 0 ? -1 : 0);
        step.z = (dir.z > 0) ? 1 : (dir.z < 0 ? -1 : 0);

        // Distance en t pour traverser un voxel
        glm::vec3 tDelta;
        tDelta.x = (dir.x != 0.0f) ? std::abs(1.0f / dir.x) : FLT_MAX;
        tDelta.y = (dir.y != 0.0f) ? std::abs(1.0f / dir.y) : FLT_MAX;
        tDelta.z = (dir.z != 0.0f) ? std::abs(1.0f / dir.z) : FLT_MAX;

        // Distance jusqu'à la première frontière
        glm::vec3 tMax;
        tMax.x = (dir.x > 0)
            ? (voxelPos.x + 1 - start.x) * tDelta.x
            : (start.x - voxelPos.x) * tDelta.x;

        tMax.y = (dir.y > 0)
            ? (voxelPos.y + 1 - start.y) * tDelta.y
            : (start.y - voxelPos.y) * tDelta.y;

        tMax.z = (dir.z > 0)
            ? (voxelPos.z + 1 - start.z) * tDelta.z
            : (start.z - voxelPos.z) * tDelta.z;

        float t = 0.0f;
        glm::ivec3 hitNormal(0);

        // Boucle DDA
        while (t <= distance)
        {
            // Récupération du chunk / bloc
            BlockId blockId = ChunkRegistry::GetBlock(voxelPos);
            if (blockId != BlockRegistry::AIR)
            {
                hit.Hit = true;
                hit.blockPose = voxelPos;
                hit.BlockType = blockId;
                hit.hitNormal = hitNormal;
                return;
            }

            // Avancer vers la frontière la plus proche
            if (tMax.x < tMax.y)
            {
                if (tMax.x < tMax.z)
                {
                    voxelPos.x += step.x;
                    t = tMax.x;
                    tMax.x += tDelta.x;
                    hitNormal = glm::ivec3(-step.x, 0, 0);
                }
                else
                {
                    voxelPos.z += step.z;
                    t = tMax.z;
                    tMax.z += tDelta.z;
                    hitNormal = glm::ivec3(0, 0, -step.z);
                }
            }
            else
            {
                if (tMax.y < tMax.z)
                {
                    voxelPos.y += step.y;
                    t = tMax.y;
                    tMax.y += tDelta.y;
                    hitNormal = glm::ivec3(0, -step.y, 0);
                }
                else
                {
                    voxelPos.z += step.z;
                    t = tMax.z;
                    tMax.z += tDelta.z;
                    hitNormal = glm::ivec3(0, 0, -step.z);
                }
            }
        }
    }
#pragma endregion

}

GameLayer::GameLayer(WorldParam param)
    : Layer()
{
    m_Param = param;

    Sunset::SaveSystem::CreateFolder(SAVE_PATH + param.Name);

    // Sunset::RenderCommande::ShowCursor(false);

    RegistryLoader::Init();
}

GameLayer::~GameLayer()
{
    glm::vec3 playerStartPosition = player.GetComponent<Sunset::TransformComponent>()->GetLocation();
    Sunset::SaveSystem::Save(SAVE_PATH + m_Param.Name + "/PlayerData.bin", playerStartPosition);

    crosshairTex.reset();
    ChunkRegistry::Destroy();
    RegistryLoader::Destroy();
}

void GameLayer::Init()
{
    Layer::Init();

    ChunkRegistry::Init(m_Param.seed, m_Param.Name, 12);
    // BlockHandDrawable = std::make_unique<Sunset::Drawable>();
    player = GetGameInstance()->m_ActiveWorld->CreateEntity("Player");
    player.AddComponent<Sunset::TransformComponent>();
    if (glm::vec3 playerStartPosition; Sunset::SaveSystem::Load(SAVE_PATH + m_Param.Name + "/PlayerData.bin", playerStartPosition))
    {
        player.GetComponent<Sunset::TransformComponent>()->SetLocation(playerStartPosition);
    }
    player.AddComponent<Sunset::NativeScriptComponent>().Bind<PlayerScript>();
    player.AddComponent<Sunset::CameraComponent>();

    constexpr glm::vec4 color{245.f/255.f, 71.f/255.f, 123.f/255.f, 1.f};

    std::uint16_t count = 64;
    auto& item = ItemRegistry::GetAll();
    for (const auto& [id, item] : item)
    {
        std::uint16_t c = count;
        m_Inventory.Add(id, c);
    }

    std::shared_ptr<SRmGUI::Panel> panel = nullptr;
    SRmGUI::SNewAssign(panel);

    glm::ivec2 winSize = Sunset::Application::GetSetting().WindowSize;

    constexpr glm::ivec2 crossSize{25, 25};
    constexpr glm::ivec2 crossHalfSize = crossSize / 2;

    crosshairTex = std::make_unique<Sunset::Texture>();
    crosshairTex->LoadImage(RESOURCES "Textures/gui/crosshair.png");

    std::shared_ptr<SRmGUI::Image> cross = nullptr;
    SRmGUI::SNewAssign(cross)
        .Position(glm::vec2{winSize.x / 2 - crossHalfSize.x, winSize.y / 2 - crossHalfSize.y})
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
    if (Sunset::NetworkService::IsInitialized())
        Sunset::NetworkService::Get().Update(dt);

    static float waterTime = 0.0f;
    waterTime += dt;
    ChunkRegistry::UpdateWaterTime(waterTime);

    GetGameInstance()->Update(dt);

    m_Inventory.Update(dt);

    glm::vec3 loc = player.GetComponent<Sunset::TransformComponent>()->GetLocation();
    ChunkRegistry::UpdatePlayerPosition(loc);

    if (prevSelectItem != currentSelectItem)
    {
        prevSelectItem = currentSelectItem;
    }

    LastTimeSaved += dt;
    if (TimeBtwSave <= LastTimeSaved)
    {
        ChunkRegistry::SaveChunk();
        LastTimeSaved = 0;
    }
}

void GameLayer::OnDraw()
{
    SS_PROFILE_FUNCTION();
    Layer::OnDraw();
    m_Sky.Draw();
    auto* cam = player.GetComponent<Sunset::CameraComponent>();
    glm::vec3 loc = player.GetComponent<Sunset::TransformComponent>()->GetLocation();
    cam->camera.SetPosition(loc);
    ChunkRegistry::DrawChunk(cam->camera);
}

bool GameLayer::OnEvent(Sunset::Event::Type &event)
{
    SS_PROFILE_FUNCTION();
    Layer::OnEvent(event);

    GetGameInstance()->m_ActiveWorld->OnEvent(event);

    // if (auto* mouseEvent = std::get_if<Sunset::Event::MouseEvent>(&event))
    // {
    //     if (mouseEvent->Scroll != 0)
    //     {
    //         currentSelectItem -= mouseEvent->Scroll;
    //         if (currentSelectItem < 0)
    //             currentSelectItem = 8;
    //         else if (currentSelectItem >= 9)
    //             currentSelectItem = 0;
    //
    //         m_Inventory.SetSelectedSlot(currentSelectItem);
    //         return true;
    //     }
    //
    //     if (mouseEvent->action == Sunset::Event::Action::Press)
    //     {
    //         RaycastHit hit;
    //         if (const auto* cam = player.GetComponent<Sunset::CameraComponent>())
    //         {
    //             glm::vec3 start = cam->camera.GetPosition();
    //             glm::vec3 forward = cam->camera.GetForward();
    //
    //             LineTrace(hit, start, forward, 10);
    //             if (!hit)
    //                 return false;
    //
    //             const glm::vec3 target = hit.blockPose + hit.hitNormal;
    //
    //             if (mouseEvent->button == 1)
    //             {
    //                 if (!m_Inventory.getSlot(currentSelectItem).Empty())
    //                 {
    //                     ChunkRegistry::SetBlock(target, ItemRegistry::Get(m_Inventory.getSlot(currentSelectItem).id).blockId);
    //                     // m_Inventory.getSlot(currentSelectItem).count--;
    //                     // if (m_Inventory.getSlot(currentSelectItem).count <= 0)
    //                     // {
    //                     //     m_Inventory.getSlot(currentSelectItem) = {Item::null, 0};
    //                     // }
    //                 }
    //             }
    //             else if (mouseEvent->button == 0)
    //                 ChunkRegistry::SetBlock(hit.blockPose, BlockRegistry::AIR);
    //         }
    //         return true;
    //     }
    // }
    return false;
}
