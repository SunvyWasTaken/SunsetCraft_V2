//
// Created by sunvy on 22/05/2026.
//

#include "Game.h"

#include <imgui.h>
#include <random>
#include <glm/gtc/type_ptr.hpp>

#include "Chunk.h"
#include "Noise.h"
#include "BaseObject/BaseCube.h"
#include "Render/Camera.h"
#include "Render/RenderCommande.h"

namespace
{
    int seed = 0;
    std::ranlux24_base rng{std::random_device{}()};
    bool MoveCamera = false;
    Sunset::Camera camera;

    bool IsDirty = false;
    bool noiseGen = false;

    int CurrentSelectedNoise = 0;

    std::vector<Chunk> chunks;
}

GameLayer::GameLayer()
{
    Noise::Init(seed);
    Sunset::InputRegister::RegisterAction("Shift",
        [&](const Sunset::Event::Action& acc)->bool
        {
            if (acc == Sunset::Event::Action::Press)
            {
                MoveCamera = !MoveCamera;
            }
            return true;
        });
    chunks.emplace_back(glm::ivec2{0, 0});
    chunks.emplace_back(glm::ivec2{SIZE_X, 0});
    chunks.emplace_back(glm::ivec2{-SIZE_X, 0});
    chunks.emplace_back(glm::ivec2{0, SIZE_Z});
    chunks.emplace_back(glm::ivec2{0, -SIZE_Z});
    chunks.emplace_back(glm::ivec2{SIZE_X, -SIZE_Z});
    chunks.emplace_back(glm::ivec2{SIZE_X, SIZE_Z});
    chunks.emplace_back(glm::ivec2{-SIZE_X, SIZE_Z});
    chunks.emplace_back(glm::ivec2{-SIZE_X, -SIZE_Z});
}

GameLayer::~GameLayer()
{
    chunks.clear();
    Noise::Destroy();
}

void GameLayer::OnUpdate(float dt)
{
    const float speed = 10.f * dt;
    if (Sunset::InputRegister::IsKeyPress("Forward"))
        camera.MoveForward(speed);
    if (Sunset::InputRegister::IsKeyPress("Backward"))
        camera.MoveBackward(speed);
    if (Sunset::InputRegister::IsKeyPress("Right"))
        camera.MoveRight(speed);
    if (Sunset::InputRegister::IsKeyPress("Left"))
        camera.MoveLeft(speed);
    if (Sunset::InputRegister::IsKeyPress("Up"))
        camera.MoveUp(speed);
    if (Sunset::InputRegister::IsKeyPress("Down"))
        camera.MoveDown(speed);

    glm::vec2 mous = Sunset::InputRegister::GetMouseDelta();
    if (MoveCamera)
    {
        if (mous.length() >= 0.1)
        {
            camera.AddPitch(-mous.y);
            camera.AddYaw(mous.x);
        }
    }

    if (!IsDirty)
        return;

    for (auto& c : chunks)
    {
        Noise::Get(c.NoiseValue, c.m_Position);
    }
    noiseGen = true;

    IsDirty = false;
}

void GameLayer::OnDraw()
{
    Sunset::RenderCommande::UseCamera(camera);

    {
        SS_PROFILE_SCOPE("Draw All Cube");
        if (noiseGen)
        {
            for (const auto& c : chunks)
                c.Draw();
        }
    }

    ImGui::Begin("Parameter");
    ImGui::InputInt("Seed", &seed);
    ImGui::SameLine();
    if (ImGui::Button("Random"))
    {
        std::uniform_int_distribution<int> dist(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
        seed = dist(rng);
        Noise::SetSeed(seed);
    }

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
        if (ImGui::Button("Add points"))
        {
            n.points.emplace_back();
        }
        for (int i = 0; i < n.points.size(); ++i)
        {
            std::string pointName = "##Point " + std::to_string(i);
            ImGui::DragFloat2(pointName.c_str(), glm::value_ptr(n.points[i]), 0.001f, -1, 1);
            ImGui::SameLine();
            std::function<void()> action;
            if (ImGui::Button(("-##" + pointName).c_str()))
            {
                action = [&](){ n.points.erase(n.points.begin() + i); };
            }
            if (action)
                action();
        }

        if(ImGui::Button("Generate"))
        {
            Noise::Update(seed);
            IsDirty = true;
        }
    }

    ImGui::End();
}
