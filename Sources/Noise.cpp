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
    std::vector<NoiseData>& GetNoiseData()
    {
        static std::vector<NoiseData> noiseData;
        return noiseData;
    }

    std::recursive_mutex noiseMutex;

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
    std::lock_guard lock(noiseMutex);
    GetNoiseData().clear();

    // Load All noiseData
    if (!Load(DefaultNoiseDataPath))
    {
        LOG("SunsetCraft", warn, "Load default noise data failed");
    }

    Update(seed);
}

void Noise::Destroy()
{
    std::lock_guard lock(noiseMutex);
    GetNoiseData().clear();
}

void Noise::SetSeed(int seed)
{
    std::lock_guard lock(noiseMutex);
    for (auto& n : GetNoiseData())
    {
        if (n.noise)
            n.noise->SetSeed(seed);
    }
}

void Noise::GetDataAt(std::vector<float> &data, int index, const glm::ivec2 &location)
{
    SS_PROFILE_FUNCTION();
    std::lock_guard lock(noiseMutex);
    data.clear();
    data.resize(SIZE_X * SIZE_Z);
    float* NoiseSet = FastNoiseSIMD::GetEmptySet(SIZE_X);
    auto& noise = GetData(index);
    for (int z = 0; z < SIZE_Z; ++z)
    {
        noise.noise->FillNoiseSet(NoiseSet, location.x, 0, location.y + z, SIZE_X, 1, 1);
        for (int x = 0; x < SIZE_X; ++x)
        {
            const int i = x + z * SIZE_X;
            data[i] = GetNoiseValue(noise.points, NoiseSet[x]) * noise.Amplitude;
        }
    }
}

void Noise::Get(std::vector<float>& data, const glm::ivec2& location)
{
    SS_PROFILE_FUNCTION();
    std::lock_guard lock(noiseMutex);
    data.clear();
    data.resize(SIZE_X * SIZE_Z);
    float* NoiseSet = FastNoiseSIMD::GetEmptySet(SIZE_X);
    for (const auto& n : GetNoiseData())
    {
        if (!n.noise)
            continue;

        for (int z = 0; z < SIZE_Z; ++z)
        {
            n.noise->FillNoiseSet(NoiseSet, location.x, 0, location.y + z, SIZE_X, 1, 1);
            for (int x = 0; x < SIZE_X; ++x)
            {
                const int i = x + z * SIZE_X;
                data[i] += GetNoiseValue(n.points, NoiseSet[x]) * n.Amplitude;
            }
        }
    }
    FastNoiseSIMD::FreeNoiseSet(NoiseSet);
}

void Noise::Update(int seed)
{
    std::lock_guard lock(noiseMutex);
    for (auto& n : GetNoiseData())
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
    std::lock_guard lock(noiseMutex);
    nlohmann::json root = nlohmann::json::array();

    for (const auto& n : GetNoiseData())
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

bool Noise::Load(const std::string& filepath)
{
    std::lock_guard lock(noiseMutex);
    std::ifstream file(SAVE_PATH + filepath);
    if (!file.is_open())
    {

        return false;
    }

    nlohmann::json root = nlohmann::json::parse(file, nullptr, false);
    if (!root.is_array())
    {
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

    GetNoiseData() = std::move(loadedNoiseData);
    return true;
}

NoiseData & Noise::GetData(int index)
{
    return GetNoiseData()[index];
}

std::vector<NoiseData> & Noise::GetDatas()
{
    return GetNoiseData();
}
