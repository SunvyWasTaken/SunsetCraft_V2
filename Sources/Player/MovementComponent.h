//
// Created by sunvy on 23/07/2026.
//

#pragma once

#include <glm/vec3.hpp>

#include "GameFramework/Components/Component.h"
#include "GameFramework/System/IWorldSystem.h"

class MovementComponent : public Sunset::Component
{
public:
    Sunset::ReflectionType Properties() override;

    void SetMoveIntent(const glm::vec3& intent);
    void RequestJump();
    void ClearFrameIntent();

    glm::vec3 MoveIntent{0.0f};
    float MaxSpeed = 10.0f;
    float JumpSpeed = 8.0f;
    bool bFlyMode = false;
    bool bJumpRequested = false;
};

class PhysicsComponent : public Sunset::Component
{
public:
    Sunset::ReflectionType Properties() override;

    glm::vec3 Velocity{0.0f};
    glm::vec3 ColliderHalfExtent{0.3f, 0.9f, 0.3f};
    glm::vec3 ColliderOffset{0.0f, -0.9f, 0.0f};
    float Gravity = -24.0f;
    float MaxFallSpeed = -60.0f;
    bool bUseGravity = true;
    bool bGrounded = false;
};

class MovementSystem : public Sunset::IWorldSystem
{
public:
    using IWorldSystem::IWorldSystem;

    void Update(float dt) override;
};
