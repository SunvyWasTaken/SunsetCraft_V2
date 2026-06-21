//
// Created by sunvy on 02/01/2026.
//

#pragma once

using BlockId = uint8_t;
class Chunk;

struct RaycastHit
{
    bool Hit = false;
    BlockId BlockType = 0;
    Chunk* chunk = nullptr;
    glm::ivec3 blockPose;
    glm::ivec3 hitNormal;

    operator bool() const
    {
        return Hit;
    }

    void Clear()
    {
        Hit = false;
        BlockType = 0;
        chunk = nullptr;
        blockPose = glm::ivec3(0);
        hitNormal = glm::ivec3(0);
    }
};
