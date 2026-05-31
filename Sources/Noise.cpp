//
// Created by sunvy on 31/05/2026.
//

#include "Noise.h"

#include "Chunk.h"

namespace
{
    std::vector<NoiseData> noiseData;

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
            return n.front().x;

        if (noise >= n.back().x)
            return n.back().x;

        for (size_t i = 0; i < n.size(); ++i)
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

NoiseData & Noise::GetData(int index)
{
    return noiseData[index];
}

std::vector<NoiseData> & Noise::GetDatas()
{
    return noiseData;
}
