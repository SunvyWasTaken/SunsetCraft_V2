//
// Created by sunvy on 22/05/2026.
//

#include "Game.h"

#include <imgui.h>
#include <random>
#include <glm/gtc/type_ptr.hpp>

#include "ChunkRegistry.h"
#include "Noise.h"
#include "GameFramework/Components/CameraComponent.h"
#include "GameFramework/Components/TransformComponent.h"
#include "Network/NetworkService.h"

namespace
{
    int seed = 0;
    std::ranlux24_base rng{std::random_device{}()};

    Sunset::Entity player;

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
    ImGui::InputInt("Seed", &seed);
    ImGui::SameLine();
    if (ImGui::Button("Random"))
    {
        std::uniform_int_distribution<int> dist(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
        seed = dist(rng);
        Noise::SetSeed(seed);
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

GameLayer::GameLayer()
{
    world = std::make_unique<Sunset::World>();
    ChunkRegistry::Init(seed, 8);
    player = world->GetController(0).GetEntity();
}

GameLayer::~GameLayer()
{
    ChunkRegistry::Destroy();
}

void GameLayer::OnUpdate(float dt)
{
    SS_PROFILE_FUNCTION();
    Sunset::NetworkService::Get().Update(dt);
    world->Update(dt);
    glm::vec3 loc = player.GetComponent<Sunset::TransformComponent>()->GetLocation();
    ChunkRegistry::UpdatePlayerPosition(loc);
}

void GameLayer::OnDraw()
{
    SS_PROFILE_FUNCTION();
    ChunkRegistry::DrawChunk(player.GetComponent<Sunset::CameraComponent>()->camera);
}
