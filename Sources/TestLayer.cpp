//
// Created by sunvy on 24/05/2026.
//

#include "TestLayer.h"

#include "BaseObject/BaseCube.h"
#include "Render/Camera.h"
#include "Render/RenderCommande.h"

namespace
{
    SunsetEngine::Camera camera;
}

TestLayer::TestLayer()
    : SunsetEngine::Layer()
{
}

TestLayer::~TestLayer()
{
}

void TestLayer::OnUpdate(float dt)
{
    const float speed = 1.f * dt;
    if (SunsetEngine::InputRegister::IsKeyPress("Forward"))
        camera.MoveForward(speed);
    if (SunsetEngine::InputRegister::IsKeyPress("Backward"))
        camera.MoveBackward(speed);
    if (SunsetEngine::InputRegister::IsKeyPress("Right"))
        camera.MoveRight(speed);
    if (SunsetEngine::InputRegister::IsKeyPress("Left"))
        camera.MoveLeft(speed);
    if (SunsetEngine::InputRegister::IsKeyPress("Up"))
        camera.MoveUp(speed);
    if (SunsetEngine::InputRegister::IsKeyPress("Down"))
        camera.MoveDown(speed);

    glm::vec2 mous = SunsetEngine::InputRegister::GetMouseDelta();
    if (mous.length() >= 0.1)
    {
        camera.AddPitch(-mous.y);
        camera.AddYaw(mous.x);
    }

    PRINTSCREEN("camera {}", camera.GetPosition())
}

void TestLayer::OnDraw()
{
    SunsetEngine::RenderCommande::UseCamera(camera);
    SunsetEngine::DrawCube({-0.5f, -0.5f, -2.f}, {1, 1, 1}, {});
    SunsetEngine::DrawCube({-1.5f, -1.5f, -3.5f}, {1, 1, 1}, {});
}
