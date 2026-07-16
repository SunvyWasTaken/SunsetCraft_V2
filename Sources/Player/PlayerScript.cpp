//
// Created by sunvy on 08/07/2026.
//

#include "PlayerScript.h"

#include "GameFramework/Components/InputComponent.h"
#include "GameFramework/Components/TransformComponent.h"
#include "../../SunsetEngine/Engine/Render/Core/RenderCommand.h"

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
        Pause,
    };

    bool ShowMouseCursor = false;
}

PlayerScript::PlayerScript()
    : ScriptEntity()
{
}

PlayerScript::~PlayerScript()
{
}

Sunset::ReflectionType PlayerScript::Properties()
{
    Sunset::ReflectionType properties;
    properties.Name = "Player";
    properties.Field("Mouvement Speed", &PlayerScript::speed);
    properties.Field("Vitesse cursor", &PlayerScript::MouseSpeed);
    return properties;
}

void PlayerScript::OnBeginPlay()
{
    ScriptEntity::OnBeginPlay();
    Sunset::RenderCommand::ShowCursor(ShowMouseCursor);
    GetComponent<Sunset::InputComponent>()->BindAction(Sunset::Key::W, MoveForward);
    GetComponent<Sunset::InputComponent>()->BindAction(Sunset::Key::S, MoveBackward);
    GetComponent<Sunset::InputComponent>()->BindAction(Sunset::Key::A, MoveLeft);
    GetComponent<Sunset::InputComponent>()->BindAction(Sunset::Key::D, MoveRight);
    GetComponent<Sunset::InputComponent>()->BindAction(Sunset::Key::E, MoveUp);
    GetComponent<Sunset::InputComponent>()->BindAction(Sunset::Key::Q, MoveDown);
    GetComponent<Sunset::InputComponent>()->BindAction(Sunset::Key::Escape, Pause);
}

void PlayerScript::OnUpdate(float dt)
{
    ScriptEntity::OnUpdate(dt);

    auto* transform = GetComponent<Sunset::TransformComponent>();
    auto* input = GetComponent<Sunset::InputComponent>();

    if (!transform || !input)
        return;

    glm::vec3 deltaPos{0, 0, 0};
    if (input->IsActionDown(MoveForward))
        deltaPos += transform->GetForwardVector();
    if (input->IsActionDown(MoveBackward))
        deltaPos -= transform->GetForwardVector();
    if (input->IsActionDown(MoveLeft))
        deltaPos -= transform->GetRightVector();
    if (input->IsActionDown(MoveRight))
        deltaPos += transform->GetRightVector();
    if (input->IsActionDown(MoveUp))
        deltaPos += glm::vec3(0, 1, 0);
    if (input->IsActionDown(MoveDown))
        deltaPos += glm::vec3(0, -1, 0);

    if (glm::length(deltaPos) > 0)
        deltaPos = glm::normalize(deltaPos);

    transform->AddLocation(deltaPos * speed * dt);

    transform->Rotate(-transform->GetRightVector(), input->MoveY() * MouseSpeed);
    transform->Rotate({0, -1, 0}, input->MoveX() * MouseSpeed);

    if (input->IsActionPressed(Pause))
    {
        ShowMouseCursor = !ShowMouseCursor;
        Sunset::RenderCommand::ShowCursor(ShowMouseCursor);
    }
}

void PlayerScript::OnEndPlay()
{
    ScriptEntity::OnEndPlay();
}
