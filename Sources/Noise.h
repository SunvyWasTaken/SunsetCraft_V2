//
// Created by sunvy on 31/05/2026.
//

#pragma once

#include "FastNoiseSIMD.h"

struct NoiseData
{
    std::string Name = "";
    int NoiseType = 0;
    int FractalOctaves = 1;

    float Frequency = 0.025f;
    int Amplitude = 24;
    std::unique_ptr<FastNoiseSIMD> noise = nullptr;
    std::vector<glm::fvec2> points{ {-1, -1}, {1, 1} };
};

struct Noise
{
    static void Init(int seed);
    static void Destroy();
    static void SetSeed(int seed);
    static void Get(std::vector<float>& data, const glm::ivec2& location);
    static void Update(int seed);
    static bool Save(const std::string& filepath);
    static bool Load(const std::string& filepath);

    static NoiseData& GetData(int index);
    static std::vector<NoiseData>& GetDatas();
};

