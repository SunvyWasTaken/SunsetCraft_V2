//
// Created by Codex on 21/07/2026.
//

#include "DayNightCycle.h"

#include <cmath>
#include <numbers>
#include <glm/geometric.hpp>

namespace
{
    float TimeOfDay = 12.0f;
    bool bAutoCycle = true;
    float TimeScale = 0.04f;

    float NormalizeTime(float timeOfDay)
    {
        timeOfDay = std::fmod(timeOfDay, 24.0f);
        if (timeOfDay < 0.0f)
            timeOfDay += 24.0f;

        return timeOfDay;
    }

}

void DayNightCycle::Update(const float dt)
{
    if (!bAutoCycle)
        return;

    SetTimeOfDay(TimeOfDay + dt * TimeScale);
}

float DayNightCycle::GetTimeOfDay()
{
    return TimeOfDay;
}

void DayNightCycle::SetTimeOfDay(const float timeOfDay)
{
    TimeOfDay = NormalizeTime(timeOfDay);
}

bool& DayNightCycle::AutoCycle()
{
    return bAutoCycle;
}

float& DayNightCycle::CycleSpeed()
{
    return TimeScale;
}

glm::vec3 DayNightCycle::GetSunDirection()
{
    const float angle = (TimeOfDay - 6.0f) / 24.0f * std::numbers::pi_v<float> * 2.0f;
    return glm::normalize(glm::vec3{std::cos(angle) * 0.35f, std::sin(angle), 0.25f});
}
