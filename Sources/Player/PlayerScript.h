//
// Created by sunvy on 08/07/2026.
//

#pragma once

#include "GameFramework/World/ScriptEntity.h"

class PlayerScript : public Sunset::ScriptEntity
{
public:
    PlayerScript();

    ~PlayerScript();

    Sunset::ReflectionType Properties() override;

    void OnBeginPlay() override;

    void OnUpdate(float dt) override;

    void OnEndPlay() override;

    float speed = 10.f;
    float MouseSpeed = 0.01f;
};
