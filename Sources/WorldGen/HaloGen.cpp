//
// Created by sunvy on 16/06/2026.
//

#include "HaloGen.h"

#include "FastNoiseSIMD.h"
#include "../Chunk.h"

struct HaloGen::Impl
{
    std::unique_ptr<FastNoiseSIMD> noise = nullptr;
    std::unique_ptr<FastNoiseSIMD>& operator()()
    {
        return noise;
    }
};

HaloGen::HaloGen()
    : impl(std::make_unique<Impl>())
{
    (*impl)() = std::unique_ptr<FastNoiseSIMD>(FastNoiseSIMD::NewFastNoiseSIMD());
    (*impl)()->SetNoiseType(FastNoiseSIMD::NoiseType::Perlin);
    (*impl)()->SetFrequency(0.0001f);
    (*impl)()->SetFractalOctaves(1);
}

HaloGen::~HaloGen()
{
}

void HaloGen::operator()(Chunk &chunk, ChunkData &data)
{
    SS_PROFILE_FUNCTION();
    (*impl)()->SetSeed(data.seed);
    float* NoiseSet = FastNoiseSIMD::GetEmptySet(SIZE_X, SIZE_Y + SIZE_Y, SIZE_Z);

    (*impl)()->FillNoiseSet(NoiseSet, chunk.m_Position.x * SIZE_X, 0, SIZE_Z, SIZE_X, SIZE_Y + SIZE_Y, SIZE_Z);
    for (size_t i = 0; i < chunk.m_Blocks.size(); ++i)
    {
        if (std::abs(NoiseSet[i]) < 0.01f)
        {
            chunk.m_Blocks[i] = BlockRegistry::AIR;
        }
    }
    FastNoiseSIMD::FreeNoiseSet(NoiseSet);
}
