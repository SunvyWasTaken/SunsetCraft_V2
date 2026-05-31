//
// Created by sunvy on 24/05/2026.
//

#include "TestLayer.h"

#include "BaseObject/BaseCube.h"
#include "Render/Camera.h"
#include "Render/RenderCommande.h"

namespace
{
    Sunset::Camera camera;
}

TestLayer::TestLayer()
    : Sunset::Layer()
{
}

TestLayer::~TestLayer()
{
}

void TestLayer::OnUpdate(float dt)
{
    const float speed = 1.f * dt;
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
    if (mous.length() >= 0.1)
    {
        camera.AddPitch(-mous.y);
        camera.AddYaw(mous.x);
    }

    PRINTSCREEN("camera {}", camera.GetPosition())
}

void TestLayer::OnDraw()
{
    Sunset::RenderCommande::UseCamera(camera);
    Sunset::DrawCube({-0.5f, -0.5f, -2.f}, {1, 1, 1}, {});
    Sunset::DrawCube({-1.5f, -1.5f, -3.5f}, {1, 1, 1}, {});
}
