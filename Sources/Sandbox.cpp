//
// Created by sunvy on 04/06/2026.
//

#include "Sandbox.h"

#include "BaseObject/BaseCube.h"
#include "GameFramework/World/World.h"
#include "Render/Camera.h"
#include "Render/RenderCommande.h"

namespace
{
    Sunset::Camera camera;
}

Sandbox::Sandbox()
{
    world = std::make_unique<Sunset::World>();
}

Sandbox::~Sandbox()
{
}

void Sandbox::OnUpdate(float deltatime)
{
    const float speed = 10.f * deltatime;
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

    // glm::vec2 mous = Sunset::InputRegister::GetMouseDelta();
    // if (mous.length() >= 0.1)
    // {
    //     camera.AddPitch(-mous.y);
    //     camera.AddYaw(mous.x);
    // }
    world->Update(deltatime);
    PRINTSCREEN("Camera location {}", camera.GetPosition());
}

void Sandbox::OnDraw()
{
    Sunset::RenderCommande::UseCamera(camera);
    //Sunset::DrawCube({0, 0, -5}, {1, 1, 1}, {});
}
