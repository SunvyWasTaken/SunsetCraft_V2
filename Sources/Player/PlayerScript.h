//
// Created by sunvy on 08/07/2026.
//

#pragma once

#include "GameFramework/World/ScriptEntity.h"

class PlayerScript : public Sunset::ScriptEntity
{
    void OnBeginPlay() override;

    void OnUpdate(float dt) override;

    void OnEndPlay() override;
};
