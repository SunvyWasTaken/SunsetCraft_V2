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

    bool IsDirty = true;
    bool noiseGen = false;
    std::vector<float> NoiseValue;
}

GameLayer::GameLayer()
{
    Noise::Init(seed);
    Sunset::InputRegister::RegisterAction("Shift", [&](const Sunset::Event::Action& acc)->bool{ if (acc == Sunset::Event::Action::Press){ MoveCamera = !MoveCamera; } return true;});
}

GameLayer::~GameLayer()
{
    NoiseValue.clear();
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

    Noise::Get(NoiseValue, {0, 0});
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
            for (int x = 0; x < SIZE_X; ++x)
            {
                for (int z = 0; z < SIZE_Z; ++z)
                {
                    float val = NoiseValue[z + x * SIZE_X];
                    for (int y = -25; y < 25; ++y)
                    {
                        if (y < val)
                        {
                            Sunset::DrawCube({x, y, z}, {}, {});
                        }
                    }
                }
            }
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

    // if(ImGui::Button("Generate"))
    // {
    //     IsDirty = true;
    // }

    ImGui::End();
}
