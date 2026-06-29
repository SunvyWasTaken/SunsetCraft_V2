//
// Created by sunvy on 22/05/2026.
//

#include "Game.h"

#include <imgui.h>
#include <random>
#include <glm/gtc/type_ptr.hpp>

#include "Registry/BlockRegistry.h"
#include "ChunkRegistry.h"
#include "HorizontalBox.h"
#include "Image.h"
#include "Noise.h"
#include "Overlay.h"
#include "RaycastHit.h"
#include "Registry/TextureRegistry.h"
#include "Core/Application.h"
#include "Core/ApplicationSetting.h"
#include "GameFramework/Components/CameraComponent.h"
#include "GameFramework/Components/TransformComponent.h"
#include "Network/NetworkService.h"
#include "Registry/ItemRegistry.h"
#include "Registry/RegistryLoader.h"
#include "Render/RenderCommande.h"
#include "Render/Texture.h"

namespace
{
    int seed = 0;
    std::ranlux24_base rng{std::random_device{}()};

    Sunset::Entity player;

    constexpr size_t ToolbarSize = 9;

    int currentSelectItem = 0;
    int prevSelectItem = 1;

    std::array<ItemStack, ToolbarSize> m_ToolBar;

    std::unique_ptr<Sunset::Drawable> BlockHandDrawable = nullptr;

    std::unique_ptr<Sunset::Texture> HotBarTexture = nullptr;

    bool ShowInventory = false;

    std::shared_ptr<SRmGUI::Overlay> Inventory = nullptr;

    std::unique_ptr<Sunset::Texture> InventoryTexture = nullptr;

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

#pragma region NoiseConfig

namespace
{
    int CurrentSelectedNoise = 0;

    bool LastNoiseSaveSucceeded = true;
    bool LastNoiseLoadSucceeded = true;

    constexpr float CurveMin = -1.f;
    constexpr float CurveMax = 1.f;
    constexpr float CurvePointRadius = 5.f;

    float ClampCurveValue(float value)
    {
        return std::clamp(value, CurveMin, CurveMax);
    }

    void SortCurvePoints(std::vector<glm::fvec2>& points)
    {
        std::sort(points.begin(), points.end(), [](const glm::fvec2& left, const glm::fvec2& right)
        {
            return left.x < right.x;
        });
    }

    ImVec2 CurvePointToScreen(const glm::fvec2& point, const ImVec2& origin, const ImVec2& size)
    {
        const float normalizedX = (point.x - CurveMin) / (CurveMax - CurveMin);
        const float normalizedY = (point.y - CurveMin) / (CurveMax - CurveMin);
        return ImVec2(origin.x + normalizedX * size.x, origin.y + (1.f - normalizedY) * size.y);
    }

    glm::fvec2 ScreenToCurvePoint(const ImVec2& position, const ImVec2& origin, const ImVec2& size)
    {
        const float normalizedX = (position.x - origin.x) / size.x;
        const float normalizedY = 1.f - ((position.y - origin.y) / size.y);
        return { ClampCurveValue(CurveMin + normalizedX * (CurveMax - CurveMin)), ClampCurveValue(CurveMin + normalizedY * (CurveMax - CurveMin)) };
    }

    int FindCurvePoint(const std::vector<glm::fvec2>& points, const ImVec2& mousePosition, const ImVec2& origin, const ImVec2& size)
    {
        int closestPoint = -1;
        float closestDistanceSquared = (CurvePointRadius * 2.f) * (CurvePointRadius * 2.f);

        for (int i = 0; i < static_cast<int>(points.size()); ++i)
        {
            const ImVec2 screenPoint = CurvePointToScreen(points[i], origin, size);
            const float distanceX = screenPoint.x - mousePosition.x;
            const float distanceY = screenPoint.y - mousePosition.y;
            const float distanceSquared = distanceX * distanceX + distanceY * distanceY;

            if (distanceSquared <= closestDistanceSquared)
            {
                closestDistanceSquared = distanceSquared;
                closestPoint = i;
            }
        }

        return closestPoint;
    }

    bool DrawCurveEditor(const char* label, std::vector<glm::fvec2>& points)
    {
        bool changed = false;
        static int draggingPoint = -1;

        SortCurvePoints(points);

        ImGui::PushID(label);
        ImGui::TextUnformatted(label);

        ImVec2 editorSize = ImVec2(ImGui::GetContentRegionAvail().x, 220.f);
        editorSize.x = std::max(editorSize.x, 280.f);

        const ImVec2 origin = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("curve_canvas", editorSize);
        const bool hovered = ImGui::IsItemHovered();
        const bool active = ImGui::IsItemActive();
        const ImVec2 mousePosition = ImGui::GetIO().MousePos;

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const ImVec2 bottomRight = ImVec2(origin.x + editorSize.x, origin.y + editorSize.y);

        drawList->AddRectFilled(origin, bottomRight, IM_COL32(28, 28, 32, 255));
        drawList->AddRect(origin, bottomRight, IM_COL32(180, 180, 180, 255));

        for (int i = 1; i < 4; ++i)
        {
            const float x = origin.x + editorSize.x * (static_cast<float>(i) / 4.f);
            const float y = origin.y + editorSize.y * (static_cast<float>(i) / 4.f);
            drawList->AddLine(ImVec2(x, origin.y), ImVec2(x, bottomRight.y), IM_COL32(70, 70, 78, 255));
            drawList->AddLine(ImVec2(origin.x, y), ImVec2(bottomRight.x, y), IM_COL32(70, 70, 78, 255));
        }

        const ImVec2 zeroXStart = CurvePointToScreen({ 0.f, CurveMin }, origin, editorSize);
        const ImVec2 zeroXEnd = CurvePointToScreen({ 0.f, CurveMax }, origin, editorSize);
        const ImVec2 zeroYStart = CurvePointToScreen({ CurveMin, 0.f }, origin, editorSize);
        const ImVec2 zeroYEnd = CurvePointToScreen({ CurveMax, 0.f }, origin, editorSize);
        drawList->AddLine(zeroXStart, zeroXEnd, IM_COL32(110, 110, 120, 255));
        drawList->AddLine(zeroYStart, zeroYEnd, IM_COL32(110, 110, 120, 255));

        if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            draggingPoint = FindCurvePoint(points, mousePosition, origin, editorSize);
            if (draggingPoint == -1)
            {
                points.emplace_back(ScreenToCurvePoint(mousePosition, origin, editorSize));
                SortCurvePoints(points);
                draggingPoint = FindCurvePoint(points, mousePosition, origin, editorSize);
                changed = true;
            }
        }

        if (active && draggingPoint >= 0 && draggingPoint < static_cast<int>(points.size()) && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            const glm::fvec2 curvePoint = ScreenToCurvePoint(mousePosition, origin, editorSize);
            points[draggingPoint] = curvePoint;
            SortCurvePoints(points);
            draggingPoint = FindCurvePoint(points, CurvePointToScreen(curvePoint, origin, editorSize), origin, editorSize);
            changed = true;
        }

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            draggingPoint = -1;
        }

        if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            const int pointToRemove = FindCurvePoint(points, mousePosition, origin, editorSize);
            if (pointToRemove >= 0 && points.size() > 2)
            {
                points.erase(points.begin() + pointToRemove);
                changed = true;
            }
        }

        for (int i = 1; i < static_cast<int>(points.size()); ++i)
        {
            drawList->AddLine(
                CurvePointToScreen(points[i - 1], origin, editorSize),
                CurvePointToScreen(points[i], origin, editorSize),
                IM_COL32(255, 190, 90, 255),
                2.f);
        }

        for (int i = 0; i < static_cast<int>(points.size()); ++i)
        {
            const ImVec2 pointPosition = CurvePointToScreen(points[i], origin, editorSize);
            drawList->AddCircleFilled(pointPosition, CurvePointRadius, IM_COL32(255, 130, 80, 255));
            drawList->AddCircle(pointPosition, CurvePointRadius, IM_COL32(255, 255, 255, 255));
        }

        if (hovered)
        {
            const glm::fvec2 hoveredPoint = ScreenToCurvePoint(mousePosition, origin, editorSize);
            ImGui::SetTooltip("Left click/drag: add or move a point\nRight click: remove a point\nX: %.3f  Y: %.3f", hoveredPoint.x, hoveredPoint.y);
        }

        ImGui::TextDisabled("Left click/drag: add or move a point. Right click: remove a point.");
        ImGui::PopID();
        return changed;
    }
}

GameOverlay::GameOverlay()
{
}

GameOverlay::~GameOverlay()
{
}

void GameOverlay::OnUpdate(float dt)
{
}

void GameOverlay::OnDraw()
{
    ImGui::Begin("Parameter");
    ImGui::Image(TextureBlockRegistry::GetTexture()->operator()(), {16* 16, 16*16});
    ImGui::InputInt("Seed", &seed);
    ImGui::SameLine();
    if (ImGui::Button("Random"))
    {
        std::uniform_int_distribution<int> dist(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
        seed = dist(rng);
        Noise::SetSeed(seed);
    }

    static int renderDistance = 12;
    if (ImGui::SliderInt("Render Distance", &renderDistance, 4, 128))
    {
        ChunkRegistry::SetRenderDistance(renderDistance);
    }

    static char noiseDataPath[128] = "NoiseData.json";
    ImGui::InputText("Noise Data File", noiseDataPath, 128);
    if (ImGui::Button("Save Noises"))
    {
        LastNoiseSaveSucceeded = Noise::Save(noiseDataPath);
    }
    ImGui::SameLine();
    if (ImGui::Button("Load Noises"))
    {
        LastNoiseLoadSucceeded = Noise::Load(noiseDataPath);
        Noise::Update(seed);
        CurrentSelectedNoise = 0;
    }
    if (!LastNoiseSaveSucceeded)
        ImGui::TextUnformatted("Save failed: cannot open file.");
    if (!LastNoiseLoadSucceeded)
        ImGui::TextUnformatted("Load failed: file not found.");

    static char name[50] = "name?";
    ImGui::InputText("Noise Name", name, 50);
    if (ImGui::Button("Add"))
    {
        Noise::GetDatas().emplace_back();
        Noise::GetDatas().back().Name = name;
    }

    if (!Noise::GetDatas().empty())
    {
        if (ImGui::BeginCombo("Noise", Noise::GetData(CurrentSelectedNoise).Name.c_str()))
        {
            for (int i = 0; i < Noise::GetDatas().size(); ++i)
            {
                bool isSelected = (CurrentSelectedNoise == i);

                if (ImGui::Selectable(Noise::GetData(i).Name.c_str(), isSelected))
                {
                    CurrentSelectedNoise = i;
                }

                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }

        NoiseData& n = Noise::GetData(CurrentSelectedNoise);

        const char* items[]{ "Value", "ValueFractal", "Perlin", "PerlinFractal", "Simplex", "SimplexFractal", "WhiteNoise", "Cellular", "Cubic", "CubicFractal" };
        ImGui::Combo("Noise Type", &n.NoiseType, items, 10);
        ImGui::InputInt("Fractal Octave", &n.FractalOctaves);
        ImGui::SliderFloat("Frequency", &n.Frequency, 0.0001f, 0.5000f);
        ImGui::SliderInt("Amplitude", &n.Amplitude, 1, 250);

        DrawCurveEditor("Noise remap curve", n.points);

        if (ImGui::TreeNode("Precise point values"))
        {
            if (ImGui::Button("Add points"))
            {
                n.points.emplace_back(0.f, 0.f);
                SortCurvePoints(n.points);
            }
            for (int i = 0; i < n.points.size(); ++i)
            {
                std::string pointName = "Point " + std::to_string(i);
                if (ImGui::DragFloat2(pointName.c_str(), glm::value_ptr(n.points[i]), 0.001f, CurveMin, CurveMax))
                {
                    n.points[i].x = ClampCurveValue(n.points[i].x);
                    n.points[i].y = ClampCurveValue(n.points[i].y);
                    SortCurvePoints(n.points);
                }
                ImGui::SameLine();
                if (ImGui::Button(("-##" + pointName).c_str()) && n.points.size() > 2)
                {
                    n.points.erase(n.points.begin() + i);
                    --i;
                }
            }
            ImGui::TreePop();
        }

        if(ImGui::Button("Generate"))
        {
            Noise::Update(seed);
        }
    }

    ImGui::End();
}

#pragma endregion // NoiseConfig

GameLayer::GameLayer()
    : Layer()
{
    world = std::make_unique<Sunset::World>();

    RegistryLoader::Init();

    ChunkRegistry::Init(seed, 12);
    // BlockHandDrawable = std::make_unique<Sunset::Drawable>();
    player = world->GetController(0).GetEntity();

    constexpr glm::vec4 color{245.f/255.f, 71.f/255.f, 123.f/255.f, 1.f};
    const glm::ivec2 WinSize = Sunset::Application::GetSetting().WindowSize;

    m_ToolBar = {ItemStack{ItemRegistry::GetId("Dirt"), 64}, {}, {}, {}, {}, {}, {}, {}, {}};

    auto panel = std::make_shared<SRmGUI::Panel>();
    auto box = std::make_shared<SRmGUI::Overlay>();
    HotBarTexture = std::make_unique<Sunset::Texture>();
    HotBarTexture->LoadImage(RESOURCES "Textures/gui/hotbar.png");
    auto HotBarImage = std::make_shared<SRmGUI::Image>();
    HotBarImage->SetImage(HotBarTexture->GetId());
    box->AddChild(HotBarImage);

    for (const auto& item : m_ToolBar)
    {
        auto overlay = std::make_shared<SRmGUI::Overlay>();
        box->AddChild(overlay);
    }

    box->SetPosition({(WinSize.x/2)-364, WinSize.y-88});
    box->SetSize({728, 88});
    panel->AddChild(box);

    InventoryTexture = std::make_unique<Sunset::Texture>();
    InventoryTexture->LoadImage(RESOURCES "Textures/Sunset.png");

    auto inv = SRmGUI::SNew<SRmGUI::Overlay>()
        .Position({WinSize.x/2 - 500, WinSize.y/2 - 500})
        .Size({500, 500})
        .Child(
            SRmGUI::SNew<SRmGUI::Image>()
            .Image(InventoryTexture->GetId())
        );

    Inventory = inv.ToShared();

    panel->AddChild(inv.ToShared());

    AddToViewport(panel);

    // ToolbarBox = std::make_unique<Sunset::HorizontalBox>();
    // ToolbarBox->SetPosition({WinSize.x/2, WinSize.y-10});
    // ToolbarBox->SetPadding({5, 0});
    // ToolbarBox->Reserve(ToolbarSize);
    // ToolbarBox->SetAnchor({0, -1});
    // for (std::uint8_t i = 0; i < ToolbarSize; ++i)
    // {
    //     ItemStack block = m_ToolBar[i];
    //     std::shared_ptr<Sunset::Slate> Square = std::make_shared<Sunset::Square>(glm::ivec2{0,0}, glm::ivec2{74, 74}, color, 15.f);
    //     ToolbarBox->AddChild(Square);
    //
    //     if (block.id != Item::null)
    //     {
    //         std::shared_ptr<Sunset::Slate> img = std::make_shared<Sunset::SlateImage>();
    //         std::static_pointer_cast<Sunset::SlateImage>(img)->LoadImage(RESOURCES "Textures/" + TextureBlockRegistry::GetTextureBlock(ItemRegistry::Get(block.id).blockId, 0));
    //         std::static_pointer_cast<Sunset::Square>(Square)->AddChild(img);
    //     }
    // }
    //
    // int length = 10;
    // int width = 4;
    // int radius = 2;
    // int spacecing = 2;
    //
    // crossTop = std::make_unique<Sunset::Square>(glm::ivec2{WinSize.x / 2, WinSize.y / 2 - spacecing}, glm::ivec2{width, length}, color, radius);
    // crossTop->SetAnchor({0, -1});
    // crossDown = std::make_unique<Sunset::Square>(glm::ivec2{WinSize.x / 2, WinSize.y / 2 + spacecing}, glm::ivec2{width, length}, color, radius);
    // crossDown->SetAnchor({0, 1});
    // crossLeft = std::make_unique<Sunset::Square>(glm::ivec2{WinSize.x / 2 - spacecing, WinSize.y / 2}, glm::ivec2{length, width}, color, radius);
    // crossLeft->SetAnchor({-1, 0});
    // crossRight = std::make_unique<Sunset::Square>(glm::ivec2{WinSize.x / 2 + spacecing, WinSize.y / 2}, glm::ivec2{length, width}, color, radius);
    // crossRight->SetAnchor({1, 0});

    Sunset::InputRegister::RegisterAction("Escape", [](const Sunset::Event::Action& action)->bool
    {
        if (action == Sunset::Event::Action::Press)
        {
            static bool ShowCursor = false;
            ShowCursor = !ShowCursor;
            Sunset::RenderCommande::ShowCursor(ShowCursor);
            return true;
        }
        return false;
    });

    Sunset::InputRegister::RegisterAction("Inventory", [](const Sunset::Event::Action& action)->bool
    {
        if (action == Sunset::Event::Action::Press)
        {
            ShowInventory = !ShowInventory;
            Inventory->SetVisibility(ShowInventory);
        }
        return true;
    });
}

GameLayer::~GameLayer()
{
    InventoryTexture.reset();
    HotBarTexture.reset();
    ChunkRegistry::Destroy();
    RegistryLoader::Destroy();
}

void GameLayer::OnUpdate(float dt)
{
    SS_PROFILE_FUNCTION();
    Layer::OnUpdate(dt);
    Sunset::NetworkService::Get().Update(dt);

    static float waterTime = 0.0f;
    waterTime += dt;
    ChunkRegistry::UpdateWaterTime(waterTime);

    world->Update(dt);
    glm::vec3 loc = player.GetComponent<Sunset::TransformComponent>()->GetLocation();
    ChunkRegistry::UpdatePlayerPosition(loc);

    if (prevSelectItem != currentSelectItem)
    {
        prevSelectItem = currentSelectItem;
    }
}

void GameLayer::OnDraw()
{
    SS_PROFILE_FUNCTION();
    Layer::OnDraw();
    m_Sky.Draw();
    if (const auto* cam = player.GetComponent<Sunset::CameraComponent>())
    {
        ChunkRegistry::DrawChunk(cam->camera);
        // Sunset::RenderCommande::Submit(*BlockHandDrawable);
    }

    if (ShowInventory)
        m_Inventory.Draw();
}

bool GameLayer::OnEvent(Sunset::Event::Type &event)
{
    SS_PROFILE_FUNCTION();
    Layer::OnEvent(event);
    if (auto* mouseEvent = std::get_if<Sunset::Event::MouseEvent>(&event))
    {
        if (mouseEvent->Scroll != 0)
        {
            currentSelectItem -= mouseEvent->Scroll;
            if (currentSelectItem < 0)
                currentSelectItem = 8;
            else if (currentSelectItem >= 9)
                currentSelectItem = 0;
            return true;
        }

        if (mouseEvent->action == Sunset::Event::Action::Press)
        {
            RaycastHit hit;
            if (const auto* cam = player.GetComponent<Sunset::CameraComponent>())
            {
                glm::vec3 start = cam->camera.GetPosition();
                glm::vec3 forward = cam->camera.GetForward();

                LineTrace(hit, start, forward, 10);
                if (!hit)
                    return false;

                const glm::vec3 target = hit.blockPose + hit.hitNormal;

                if (mouseEvent->button == 1)
                {
                    ChunkRegistry::SetBlock(target, ItemRegistry::Get(m_ToolBar[currentSelectItem].id).blockId);
                    m_ToolBar[currentSelectItem].count--;
                    if (m_ToolBar[currentSelectItem].count <= 0)
                    {
                         m_ToolBar[currentSelectItem] = {};
                    }
                }
                else if (mouseEvent->button == 0)
                    ChunkRegistry::SetBlock(hit.blockPose, BlockRegistry::AIR);
            }
            return true;
        }
    }
    return false;
}
