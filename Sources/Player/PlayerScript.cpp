//
// Created by sunvy on 08/07/2026.
//

#include "PlayerScript.h"

#include "ChunkRegistry.h"
#include "RaycastHit.h"
#include "GameFramework/Components/InputComponent.h"
#include "GameFramework/Components/TransformComponent.h"
#include "../../SunsetEngine/Engine/Render/Core/RenderCommand.h"
#include "GameFramework/Components/CameraComponent.h"
#include "Registry/BlockRegistry.h"
#include "Registry/ItemRegistry.h"

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
        MainAction,
        SecondaryAction,
    };

    bool ShowMouseCursor = false;

#pragma region LineTrace
    void LineTrace(RaycastHit& hit, ChunkRegistry* chunk, const glm::vec3& start, const glm::vec3& forward, float distance)
    {
        hit.Clear();

        // Direction du rayon
        glm::vec3 dir = glm::normalize(forward);

        // Position voxel courante
        glm::ivec3 voxelPos = glm::floor(start);

        // Sens de progression
        glm::ivec3 step;
        step.x = (dir.x > 0) ? 1 : (dir.x < 0 ? -1 : 0);
        step.y = (dir.y > 0) ? 1 : (dir.y < 0 ? -1 : 0);
        step.z = (dir.z > 0) ? 1 : (dir.z < 0 ? -1 : 0);

        // Distance en t pour traverser un voxel
        glm::vec3 tDelta;
        tDelta.x = (dir.x != 0.0f) ? std::abs(1.0f / dir.x) : FLT_MAX;
        tDelta.y = (dir.y != 0.0f) ? std::abs(1.0f / dir.y) : FLT_MAX;
        tDelta.z = (dir.z != 0.0f) ? std::abs(1.0f / dir.z) : FLT_MAX;

        // Distance jusqu'à la première frontière
        glm::vec3 tMax;
        tMax.x = (dir.x > 0)
            ? (voxelPos.x + 1 - start.x) * tDelta.x
            : (start.x - voxelPos.x) * tDelta.x;

        tMax.y = (dir.y > 0)
            ? (voxelPos.y + 1 - start.y) * tDelta.y
            : (start.y - voxelPos.y) * tDelta.y;

        tMax.z = (dir.z > 0)
            ? (voxelPos.z + 1 - start.z) * tDelta.z
            : (start.z - voxelPos.z) * tDelta.z;

        float t = 0.0f;
        glm::ivec3 hitNormal(0);

        // Boucle DDA
        while (t <= distance)
        {
            // Récupération du chunk / bloc
            BlockId blockId = chunk->GetBlock(voxelPos);
            if (blockId != BlockRegistry::AIR)
            {
                hit.Hit = true;
                hit.blockPose = voxelPos;
                hit.BlockType = blockId;
                hit.hitNormal = hitNormal;
                return;
            }

            // Avancer vers la frontière la plus proche
            if (tMax.x < tMax.y)
            {
                if (tMax.x < tMax.z)
                {
                    voxelPos.x += step.x;
                    t = tMax.x;
                    tMax.x += tDelta.x;
                    hitNormal = glm::ivec3(-step.x, 0, 0);
                }
                else
                {
                    voxelPos.z += step.z;
                    t = tMax.z;
                    tMax.z += tDelta.z;
                    hitNormal = glm::ivec3(0, 0, -step.z);
                }
            }
            else
            {
                if (tMax.y < tMax.z)
                {
                    voxelPos.y += step.y;
                    t = tMax.y;
                    tMax.y += tDelta.y;
                    hitNormal = glm::ivec3(0, -step.y, 0);
                }
                else
                {
                    voxelPos.z += step.z;
                    t = tMax.z;
                    tMax.z += tDelta.z;
                    hitNormal = glm::ivec3(0, 0, -step.z);
                }
            }
        }
    }
#pragma endregion
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
    GetComponent<Sunset::InputComponent>()->BindAction(Sunset::MouseKey::Left, MainAction);
    GetComponent<Sunset::InputComponent>()->BindAction(Sunset::MouseKey::Right, SecondaryAction);
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

    // auto* chunk = GetComponent<ChunkRegistry>();
    // if (const auto* cam = GetComponent<Sunset::CameraComponent>())
    // {
    //     RaycastHit hit;
    //     glm::vec3 start = cam->camera.GetPosition();
    //     glm::vec3 forward = cam->camera.GetForward();
    //
    //     LineTrace(hit, chunk, start, forward, 10);
    //     if (!hit)
    //         return;
    //
    //
    //     if (input->IsActionPressed(SecondaryAction))
    //     {
    //         const glm::vec3 target = hit.blockPose + hit.hitNormal;
    //
    //         if (!m_Inventory.getSlot(currentSelectItem).Empty())
    //         {
    //             ChunkRegistry::SetBlock(target, ItemRegistry::Get(m_Inventory.getSlot(currentSelectItem).id).blockId);
    //             m_Inventory.getSlot(currentSelectItem).count--;
    //             if (m_Inventory.getSlot(currentSelectItem).count <= 0)
    //             {
    //                 m_Inventory.getSlot(currentSelectItem) = {Item::null, 0};
    //             }
    //         }
    //     }
    //     else if (input->IsActionPressed(MainAction))
    //         chunk->SetBlock(hit.blockPose, BlockRegistry::AIR);
    // }
}

void PlayerScript::OnEndPlay()
{
    ScriptEntity::OnEndPlay();
}
