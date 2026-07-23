//
// Created by sunvy on 23/07/2026.
//

#include "Player/MovementComponent.h"

#include <algorithm>
#include <cmath>

#include <glm/common.hpp>
#include <glm/geometric.hpp>

#include "ChunkRegistry.h"
#include "GameFramework/Components/TransformComponent.h"
#include "GameFramework/World/Entity.h"
#include "GameFramework/World/World.h"
#include "Registry/BlockRegistry.h"

namespace
{
    constexpr float SkinWidth = 0.001f;
    constexpr float MaxMoveStep = 0.25f;

    struct PhysicsAABB
    {
        glm::vec3 Min{0.0f};
        glm::vec3 Max{0.0f};
    };

    PhysicsAABB GetAABB(const glm::vec3& position, const PhysicsComponent& physics)
    {
        const glm::vec3 center = position + physics.ColliderOffset;
        return {
            center - physics.ColliderHalfExtent,
            center + physics.ColliderHalfExtent
        };
    }

    bool IsSolidBlock(const BlockId blockId)
    {
        return blockId != BlockRegistry::AIR && blockId != BlockRegistry::WATER;
    }

    bool CollidesWithChunk(ChunkRegistry& chunk, const PhysicsAABB& aabb)
    {
        const glm::ivec3 minBlock = glm::floor(aabb.Min + glm::vec3(SkinWidth));
        const glm::ivec3 maxBlock = glm::floor(aabb.Max - glm::vec3(SkinWidth));

        for (int x = minBlock.x; x <= maxBlock.x; ++x)
        {
            for (int y = minBlock.y; y <= maxBlock.y; ++y)
            {
                for (int z = minBlock.z; z <= maxBlock.z; ++z)
                {
                    if (IsSolidBlock(chunk.GetBlock({x, y, z})))
                        return true;
                }
            }
        }

        return false;
    }

    bool MoveAxis(Sunset::TransformComponent& transform, PhysicsComponent& physics, ChunkRegistry& chunk, const int axis, const float distance)
    {
        if (std::abs(distance) <= 0.0f)
            return false;

        glm::vec3 position = transform.GetLocation();
        position[axis] += distance;

        if (!CollidesWithChunk(chunk, GetAABB(position, physics)))
        {
            transform.SetLocation(position);
            return false;
        }

        const PhysicsAABB aabb = GetAABB(position, physics);
        if (distance > 0.0f)
        {
            const int blockMin = static_cast<int>(std::floor(aabb.Max[axis]));
            position[axis] = static_cast<float>(blockMin)
                - physics.ColliderOffset[axis]
                - physics.ColliderHalfExtent[axis]
                - SkinWidth;
        }
        else
        {
            const int blockMax = static_cast<int>(std::floor(aabb.Min[axis])) + 1;
            position[axis] = static_cast<float>(blockMax)
                - physics.ColliderOffset[axis]
                + physics.ColliderHalfExtent[axis]
                + SkinWidth;
        }

        transform.SetLocation(position);
        return true;
    }

    void ApplyMovement(const float dt, Sunset::TransformComponent& transform, MovementComponent& movement, PhysicsComponent& physics, ChunkRegistry& chunk)
    {
        glm::vec3 velocity = physics.Velocity;

        if (movement.bFlyMode)
        {
            velocity = movement.MoveIntent * movement.MaxSpeed;
            physics.bGrounded = false;
        }
        else
        {
            velocity.x = movement.MoveIntent.x * movement.MaxSpeed;
            velocity.z = movement.MoveIntent.z * movement.MaxSpeed;

            if (movement.bJumpRequested && physics.bGrounded)
            {
                velocity.y = movement.JumpSpeed;
                physics.bGrounded = false;
            }

            if (physics.bUseGravity)
                velocity.y = std::max(velocity.y + physics.Gravity * dt, physics.MaxFallSpeed);
        }

        physics.Velocity = velocity;

        const glm::vec3 delta = physics.Velocity * dt;
        const float longestAxis = std::max({std::abs(delta.x), std::abs(delta.y), std::abs(delta.z)});
        const int steps = std::max(1, static_cast<int>(std::ceil(longestAxis / MaxMoveStep)));
        const glm::vec3 stepDelta = delta / static_cast<float>(steps);

        for (int i = 0; i < steps; ++i)
        {
            if (MoveAxis(transform, physics, chunk, 0, stepDelta.x))
                physics.Velocity.x = 0.0f;

            const bool hitY = MoveAxis(transform, physics, chunk, 1, stepDelta.y);
            if (hitY)
            {
                physics.bGrounded = stepDelta.y < 0.0f;
                physics.Velocity.y = 0.0f;
            }
            else if (stepDelta.y < 0.0f)
            {
                physics.bGrounded = false;
            }

            if (MoveAxis(transform, physics, chunk, 2, stepDelta.z))
                physics.Velocity.z = 0.0f;
        }

        movement.ClearFrameIntent();
    }
}

Sunset::ReflectionType MovementComponent::Properties()
{
    Sunset::ReflectionType type;
    type.Name = "Movement";
    type.Field("Move Intent", &MovementComponent::MoveIntent);
    type.Field("Max Speed", &MovementComponent::MaxSpeed);
    type.Field("Jump Speed", &MovementComponent::JumpSpeed);
    type.Field("Fly Mode", &MovementComponent::bFlyMode);
    return type;
}

void MovementComponent::SetMoveIntent(const glm::vec3& intent)
{
    MoveIntent = glm::length(intent) > 1.0f ? glm::normalize(intent) : intent;
}

void MovementComponent::RequestJump()
{
    bJumpRequested = true;
}

void MovementComponent::ClearFrameIntent()
{
    MoveIntent = glm::vec3{0.0f};
    bJumpRequested = false;
}

Sunset::ReflectionType PhysicsComponent::Properties()
{
    Sunset::ReflectionType type;
    type.Name = "Physics";
    type.Field("Velocity", &PhysicsComponent::Velocity);
    type.Field("Collider Half Extent", &PhysicsComponent::ColliderHalfExtent);
    type.Field("Collider Offset", &PhysicsComponent::ColliderOffset);
    type.Field("Gravity", &PhysicsComponent::Gravity);
    type.Field("Max Fall Speed", &PhysicsComponent::MaxFallSpeed);
    type.Field("Use Gravity", &PhysicsComponent::bUseGravity);
    type.Field("Grounded", &PhysicsComponent::bGrounded);
    return type;
}

void MovementSystem::Update(const float dt)
{
    IWorldSystem::Update(dt);

    m_World->Each<Sunset::TransformComponent, MovementComponent, PhysicsComponent, ChunkRegistry>(
        [&](const Sunset::Entity& entity, Sunset::TransformComponent& transform, MovementComponent& movement, PhysicsComponent& physics, ChunkRegistry& chunk)
        {
            (void)entity;
            if (!chunk.IsChunkLoadedAt(transform.GetLocation()))
            {
                movement.ClearFrameIntent();
                return;
            }

            ApplyMovement(dt, transform, movement, physics, chunk);
        });
}
