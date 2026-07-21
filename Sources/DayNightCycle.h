//
// Created by Codex on 21/07/2026.
//

#pragma once

#include <glm/vec3.hpp>

struct DayNightCycle
{
    static void Update(float dt);

    static float GetTimeOfDay();
    static void SetTimeOfDay(float timeOfDay);

    static bool& AutoCycle();
    static float& CycleSpeed();

    static float GetRenderTime();
    static glm::vec3 GetSunDirection();
};
