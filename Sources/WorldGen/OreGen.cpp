//
// Created by sunvy on 21/06/2026.
//

#include "OreGen.h"

#include "FastNoiseSIMD.h"
#include "../Chunk.h"

namespace
{
    struct OreVein
    {
        BlockId block = BlockRegistry::AIR;
        int minY = -SIZE_Y;
        int maxY = SIZE_Y - 1;
        float frequency = 0.05f;
        float threshold = 0.65f;
        int seedOffset = 0;
    };

    float DepthFactor(const int y, const OreVein& vein)
    {
        if (vein.maxY == vein.minY)
            return 1.f;

        const float t = static_cast<float>(y - vein.minY) / static_cast<float>(vein.maxY - vein.minY);
        return std::clamp(t, 0.f, 1.f);
    }

    void FillOreVein(GeneratedChunk& chunk, const GenerationData& data, const OreVein& vein)
    {
        std::unique_ptr<FastNoiseSIMD> noise(FastNoiseSIMD::NewFastNoiseSIMD(data.seed + vein.seedOffset));
        noise->SetNoiseType(FastNoiseSIMD::NoiseType::SimplexFractal);
        noise->SetFrequency(vein.frequency);
        noise->SetFractalOctaves(3);

        float* noiseSet = FastNoiseSIMD::GetEmptySet(SIZE_Y + SIZE_Y);

        for (int x = 0; x < SIZE_X; ++x)
        {
            for (int z = 0; z < SIZE_Z; ++z)
            {
                const int worldX = chunk.position.x * SIZE_X + x;
                const int worldZ = chunk.position.y * SIZE_Z + z;
                noise->FillNoiseSet(noiseSet, worldX, -SIZE_Y, worldZ, 1, SIZE_Y + SIZE_Y, 1);

                for (int y = vein.minY; y <= vein.maxY; ++y)
                {
                    const size_t index = Index(x, y, z);
                    if (chunk.blocks[index] != BlockRegistry::STONE)
                        continue;

                    const float layerBonus = 0.08f * (1.f - std::abs((DepthFactor(y, vein) * 2.f) - 1.f));
                    if (noiseSet[y + SIZE_Y] > vein.threshold - layerBonus)
                        chunk.blocks[index] = vein.block;
                }
            }
        }

        FastNoiseSIMD::FreeNoiseSet(noiseSet);
    }
}

void OreGen::operator()(GeneratedChunk &chunk, GenerationData &data)
{
    SS_PROFILE_FUNCTION();
    OreVein coal;
    coal.block = BlockRegistry::Get("coal_ore");
    coal.minY = -32;
    coal.maxY = 128;
    coal.frequency = 0.045f;
    coal.threshold = 0.58f;
    coal.seedOffset = 401;

    OreVein diamond;
    diamond.block = BlockRegistry::Get("diamond_ore");
    diamond.minY = -SIZE_Y;
    diamond.maxY = -SIZE_Y + 20;
    diamond.frequency = 0.0195f;
    diamond.threshold = 0.774f;
    diamond.seedOffset = 997;

    FillOreVein(chunk, data, coal);
    FillOreVein(chunk, data, diamond);
}
