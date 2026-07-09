//
// Created by sunvy on 08/07/2026.
//

#include "PlayerScript.h"

#include "GameFramework/Components/InputComponent.h"

namespace
{
    enum PlayerAction : Sunset::InputAction
    {
        MoveX,
        MoveY
    };
}

void PlayerScript::OnBeginPlay()
{
    ScriptEntity::OnBeginPlay();
}

void PlayerScript::OnUpdate(float dt)
{
    ScriptEntity::OnUpdate(dt);

    if (auto* input = GetComponent<Sunset::InputComponent>())
    {
        if (input->IsActionDown(PlayerAction::MoveX))
        {

        }
    }
}

void PlayerScript::OnEndPlay()
{
    ScriptEntity::OnEndPlay();
}
