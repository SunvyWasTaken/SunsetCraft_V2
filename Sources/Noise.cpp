//
// Created by sunvy on 31/05/2026.
//

#include "Noise.h"

#include <fstream>
#include <utility>
#include <nlohmann/json.hpp>

#include "Chunk.h"

namespace
{
    std::vector<NoiseData> noiseData;

    constexpr const char* DefaultNoiseDataPath = "NoiseData.json";

    FastNoiseSIMD::NoiseType ItoNoise(const int i)
    {
        switch (i)
        {
            case 0:
                return FastNoiseSIMD::NoiseType::Value;
            case 1:
                return FastNoiseSIMD::NoiseType::ValueFractal;
            case 2:
                return FastNoiseSIMD::NoiseType::Perlin;
            case 3:
                return FastNoiseSIMD::NoiseType::PerlinFractal;
            case 4:
                return FastNoiseSIMD::NoiseType::Simplex;
            case 5:
                return FastNoiseSIMD::NoiseType::SimplexFractal;
            case 6:
                return FastNoiseSIMD::NoiseType::WhiteNoise;
            case 7:
                return FastNoiseSIMD::NoiseType::Cellular;
            case 8:
                return FastNoiseSIMD::NoiseType::Cubic;
            case 9:
                return FastNoiseSIMD::NoiseType::CubicFractal;
            default:
                return FastNoiseSIMD::NoiseType::Value;
        }
    }

    float GetNoiseValue(const std::vector<glm::fvec2>& n, float noise)
    {
        if (n.empty())
            return 0.f;

        if (noise <= n.front().x)
            return n.front().y;

        if (noise >= n.back().x)
            return n.back().y;

        for (size_t i = 0; i + 1 < n.size(); ++i)
        {
            const glm::vec2 p1 = n[i];
            const glm::vec2 p2 = n[i + 1];

            if (noise >= p1.x && noise <= p2.x)
            {
                float t = (noise - p1.x) / (p2.x - p1.x);
                return p1.y + t * (p2.y - p1.y);
            }
        }
        return 0.f;
    }
}

void Noise::Init(int seed)
{
    noiseData.clear();

    // Load All noiseData

    Update(seed);
}

void Noise::Destroy()
{
    noiseData.clear();
}

void Noise::SetSeed(int seed)
{
    for (auto& n : noiseData)
    {
        if (n.noise)
            n.noise->SetSeed(seed);
    }
}

void Noise::Get(std::vector<float>& data, const glm::ivec2& location)
{
    data.clear();
    data.resize(SIZE_X * SIZE_Z);
    float* NoiseSet = FastNoiseSIMD::GetEmptySet(SIZE_X * SIZE_Z);
    for (const auto& n : noiseData)
    {
        if (!n.noise)
            continue;

        n.noise->FillNoiseSet(NoiseSet, location.x, location.y, 0, SIZE_X, SIZE_Z, 1);
        for (int i = 0; i < SIZE_X * SIZE_Z; ++i)
        {
            data[i] += GetNoiseValue(n.points, NoiseSet[i]) * n.Amplitude;
        }
    }
    FastNoiseSIMD::FreeNoiseSet(NoiseSet);
}

void Noise::Update(int seed)
{
    for (auto& n : noiseData)
    {
        std::sort(n.points.begin(), n.points.end(), [](const glm::fvec2& left, const glm::fvec2& right)
        {
            return left.x < right.x;
        });
        n.noise = std::unique_ptr<FastNoiseSIMD>(FastNoiseSIMD::NewFastNoiseSIMD(seed));
        n.noise->SetNoiseType(ItoNoise(n.NoiseType));
        n.noise->SetFractalOctaves(n.FractalOctaves);
        n.noise->SetFrequency(n.Frequency);
    }
}

bool Noise::Save(const std::string& filepath)
{
    nlohmann::json root = nlohmann::json::array();

    for (const auto& n : noiseData)
    {
        nlohmann::json points = nlohmann::json::array();
        for (const auto& point : n.points)
        {
            points.push_back({ { "x", point.x }, { "y", point.y } });
        }

        root.push_back({
            { "name", n.Name },
            { "noiseType", n.NoiseType },
            { "fractalOctaves", n.FractalOctaves },
            { "frequency", n.Frequency },
            { "amplitude", n.Amplitude },
            { "points", points }
        });
    }

    std::ofstream file(SAVE_PATH + filepath);
    if (!file.is_open())
        return false;

    file << root.dump(4);
    return true;
}

bool Noise::Load(const std::string& filepath, int seed)
{
    std::ifstream file(SAVE_PATH + filepath);
    if (!file.is_open())
    {
        Update(seed);
        return false;
    }

    nlohmann::json root = nlohmann::json::parse(file, nullptr, false);
    if (!root.is_array())
    {
        Update(seed);
        return false;
    }

    std::vector<NoiseData> loadedNoiseData;
    for (const auto& item : root)
    {
        if (!item.is_object())
            continue;

        NoiseData data;
        data.Name = item.value("name", "");
        data.NoiseType = item.value("noiseType", 0);
        data.FractalOctaves = item.value("fractalOctaves", 1);
        data.Frequency = item.value("frequency", 0.025f);
        data.Amplitude = item.value("amplitude", 24);
        data.points.clear();

        for (const auto& point : item.value("points", nlohmann::json::array()))
        {
            if (!point.is_object())
                continue;

            data.points.emplace_back(point.value("x", 0.f), point.value("y", 0.f));
        }

        if (data.points.empty())
        {
            data.points = { {-1.f, -1.f}, {1.f, 1.f} };
        }

        loadedNoiseData.emplace_back(std::move(data));
    }

    noiseData = std::move(loadedNoiseData);
    Update(seed);
    return true;
}

NoiseData & Noise::GetData(int index)
{
    return noiseData[index];
}

std::vector<NoiseData> & Noise::GetDatas()
{
    return noiseData;
}
