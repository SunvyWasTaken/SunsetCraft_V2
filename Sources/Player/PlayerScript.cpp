//
// Created by sunvy on 08/07/2026.
//

#include "PlayerScript.h"

#include "GameFramework/Components/InputComponent.h"
#include "GameFramework/Components/TransformComponent.h"
#include "Render/RenderCommande.h"

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
    LOG("SunsetCraft", info, "Player Script Creation");
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
    Sunset::RenderCommande::ShowCursor(ShowMouseCursor);
    GetComponent<Sunset::InputComponent>()->BindAction(Sunset::Key::W, PlayerAction::MoveForward);
    GetComponent<Sunset::InputComponent>()->BindAction(Sunset::Key::S, PlayerAction::MoveBackward);
    GetComponent<Sunset::InputComponent>()->BindAction(Sunset::Key::A, PlayerAction::MoveLeft);
    GetComponent<Sunset::InputComponent>()->BindAction(Sunset::Key::D, PlayerAction::MoveRight);
    GetComponent<Sunset::InputComponent>()->BindAction(Sunset::Key::E, PlayerAction::MoveUp);
    GetComponent<Sunset::InputComponent>()->BindAction(Sunset::Key::Q, PlayerAction::MoveDown);
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
    if (input->IsActionDown(PlayerAction::MoveForward))
        deltaPos += transform->GetForwardVector();
    if (input->IsActionDown(PlayerAction::MoveBackward))
        deltaPos -= transform->GetForwardVector();
    if (input->IsActionDown(PlayerAction::MoveLeft))
        deltaPos -= transform->GetRightVector();
    if (input->IsActionDown(PlayerAction::MoveRight))
        deltaPos += transform->GetRightVector();
    if (input->IsActionDown(PlayerAction::MoveUp))
        deltaPos += glm::vec3(0, 1, 0);
    if (input->IsActionDown(PlayerAction::MoveDown))
        deltaPos += glm::vec3(0, -1, 0);

    if (glm::length(deltaPos) > 0)
        deltaPos = glm::normalize(deltaPos);

    transform->AddLocation(deltaPos * speed * dt);

    transform->Rotate(-transform->GetRightVector(), input->MoveY() * MouseSpeed);
    transform->Rotate({0, -1, 0}, input->MoveX() * MouseSpeed);

    if (input->IsActionPressed(Pause))
    {
        ShowMouseCursor = !ShowMouseCursor;
        Sunset::RenderCommande::ShowCursor(ShowMouseCursor);
    }
}

void PlayerScript::OnEndPlay()
{
    ScriptEntity::OnEndPlay();
}
