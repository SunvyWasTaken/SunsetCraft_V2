//
// Created by sunvy on 08/07/2026.
//

#include "PlayerScript.h"

#include "GameFramework/Components/InputComponent.h"
#include "GameFramework/Components/TransformComponent.h"

namespace
{
    enum PlayerAction : Sunset::InputAction
    {
        MoveForward,
        MoveBackward,
        MoveLeft,
        MoveRight,
        MoveUp,
        MoveDown,
    };

    glm::vec2 prevPos{0, 0};
}

void PlayerScript::OnBeginPlay()
{
    ScriptEntity::OnBeginPlay();
    GetComponent<Sunset::InputComponent>()->BindAction(Sunset::Key::W, PlayerAction::MoveForward);
    GetComponent<Sunset::InputComponent>()->BindAction(Sunset::Key::S, PlayerAction::MoveBackward);
    GetComponent<Sunset::InputComponent>()->BindAction(Sunset::Key::A, PlayerAction::MoveLeft);
    GetComponent<Sunset::InputComponent>()->BindAction(Sunset::Key::D, PlayerAction::MoveRight);
    GetComponent<Sunset::InputComponent>()->BindAction(Sunset::Key::E, PlayerAction::MoveUp);
    GetComponent<Sunset::InputComponent>()->BindAction(Sunset::Key::Q, PlayerAction::MoveDown);
}

void PlayerScript::OnUpdate(float dt)
{
    ScriptEntity::OnUpdate(dt);

    auto* transform = GetComponent<Sunset::TransformComponent>();

    if (!transform)
        return;

    if (auto* input = GetComponent<Sunset::InputComponent>())
    {
        glm::vec3 deltaPos{0, 0, 0};
        if (input->IsActionDown(PlayerAction::MoveForward))
            deltaPos += transform->GetForwardVector() * speed * dt;
        if (input->IsActionDown(PlayerAction::MoveBackward))
            deltaPos -= transform->GetForwardVector() * speed * dt;
        if (input->IsActionDown(PlayerAction::MoveLeft))
            deltaPos -= transform->GetRightVector() * speed * dt;
        if (input->IsActionDown(PlayerAction::MoveRight))
            deltaPos += transform->GetRightVector() * speed * dt;
        if (input->IsActionDown(PlayerAction::MoveUp))
            deltaPos += glm::vec3(0, 1, 0) * speed * dt;
        if (input->IsActionDown(PlayerAction::MoveDown))
            deltaPos += glm::vec3(0, -1, 0) * speed * dt;

        if (glm::length(deltaPos) > 0)
            deltaPos = glm::normalize(deltaPos);

        transform->Position += deltaPos;
        auto currPos = input->GetInputSystem().GetMousePosition();
        auto delta = currPos - prevPos;
        prevPos = currPos;
        if (delta.x > 0)
            transform->Rotate(transform->GetRightVector(), -delta.y * 0.05f);
        if (input->MoveY() > 0)
            transform->Rotate({0, 1, 0}, delta.x * 0.05f);
    }
}

void PlayerScript::OnEndPlay()
{
    ScriptEntity::OnEndPlay();
}
