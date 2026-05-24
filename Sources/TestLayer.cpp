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
    const float speed = 100.f * dt;
    if (SunsetEngine::InputRegister::IsKeyPress("Forward"))
        camera.MoveForward(speed);
    if (SunsetEngine::InputRegister::IsKeyPress("Backward"))
        camera.MoveBackward(speed);
    if (SunsetEngine::InputRegister::IsKeyPress("Right"))
        camera.MoveRight(speed);
    if (SunsetEngine::InputRegister::IsKeyPress("Left"))
        camera.MoveLeft(speed);
}

void TestLayer::OnDraw()
{
    SunsetEngine::RenderCommande::UseCamera(camera);
    SunsetEngine::DrawCube({1.f, 1.f, 1.f}, {}, {});
}
