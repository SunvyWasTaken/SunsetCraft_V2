//
// Created by sunvy on 31/05/2026.
//

#pragma once

struct Noise
{
    static void Init(int seed);
    static void Destroy();
    static void SetSeed(int seed);
    static void Get(std::vector<float>& data, const glm::ivec2& location);
};

