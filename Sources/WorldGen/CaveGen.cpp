//
// Created by sunvy on 16/06/2026.
//

#include "CaveGen.h"

#include "FastNoiseSIMD.h"
#include "../Chunk.h"

namespace
{
    template <typename T>
    requires std::is_arithmetic_v<T>
    [[nodiscard]]float Normalize(T min, T max, T value)
    {
        return static_cast<float>(value - min) / static_cast<float>(max - min);
    }

    float Lerp(float a, float b, float t)
    {
        return a + t * (b - a);
    }

    float EaseInOutQuad(float t)
    {
        if (t < 0.5f)
            return 2.0f * t * t;

        return 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) * 0.5f;
    }

    float EaseInOutLerp(float a, float b, float t)
    {
        return Lerp(a, b, EaseInOutQuad(t));
    }
}

struct CaveGen::Impl
{
    std::unique_ptr<FastNoiseSIMD> noise = nullptr;
    std::unique_ptr<FastNoiseSIMD>& operator()()
    {
        return noise;
    }
};

CaveGen::CaveGen()
    : impl(std::make_unique<Impl>())
{
    (*impl)() = std::unique_ptr<FastNoiseSIMD>(FastNoiseSIMD::NewFastNoiseSIMD());
    (*impl)()->SetNoiseType(FastNoiseSIMD::NoiseType::ValueFractal);
    (*impl)()->SetFrequency(0.05f);
    (*impl)()->SetFractalOctaves(3);
}

CaveGen::~CaveGen()
{
}

void CaveGen::operator()(Chunk &chunk, ChunkData &data)
{
    SS_PROFILE_FUNCTION();
    (*impl)()->SetSeed(data.seed);
    float* NoiseSet = FastNoiseSIMD::GetEmptySet(SIZE_Y + SIZE_Y);

    for (int x = 0; x < SIZE_X; ++x)
    {
        for (int z = 0; z < SIZE_Z; ++z)
        {
            (*impl)()->FillNoiseSet(NoiseSet, chunk.m_Position.x * SIZE_X + x, -SIZE_Y, chunk.m_Position.y * SIZE_Z + z, 1, SIZE_Y + SIZE_Y, 1);
            for (int y = -SIZE_Y; y < SIZE_Y; ++y)
            {
                if (NoiseSet[y + SIZE_Y] < -0.3f)
                {
                    chunk.m_Blocks[x + z * SIZE_X + (y + SIZE_Y) * SIZE_X * SIZE_Z] = BlockRegistry::AIR;
                }
            }
        }
    }
    FastNoiseSIMD::FreeNoiseSet(NoiseSet);
}
