//
// Created by sunvy on 20/06/2026.
//

#include "LandGen.h"

#include "FastNoiseSIMD.h"

namespace
{
    constexpr float TreeFrequency = 0.1f;
    constexpr float TreeThreshhold = 0.1f;

    void PlaceTree(GeneratedChunk &chunkData, int x, int y, int z)
    {
        const BlockId wood  = BlockRegistry::Get("wood");
        const BlockId leaves = BlockRegistry::Get("oak_leaf");

        if (wood == BlockRegistry::AIR || leaves == BlockRegistry::AIR)
            return;

        const int trunkHeight = 3;

        int topY = y + trunkHeight;

        // --- TRONC ---
        for (int i = 1; i <= trunkHeight; ++i)
        {
            int localY = y + i;
            if (localY >= SIZE_Y)
                break;

            chunkData.blocks[Index(x, localY, z)] = wood;
        }

        // --- FEUILLES ---
        // Layers du haut vers le bas
        for (int layer = 0; layer < 4; ++layer)
        {
            int currentY = topY - layer;

            // Rayon variable selon la hauteur
            int radius = 0;
            if (layer == 0) radius = 1;        // sommet
            else if (layer == 1) radius = 2;
            else if (layer == 2) radius = 2;
            else radius = 1;                   // base

            for (int dz = -radius; dz <= radius; ++dz)
            {
                for (int dx = -radius; dx <= radius; ++dx)
                {
                    int lx = x + dx;
                    int ly = currentY;
                    int lz = z + dz;

                    if (lx < 0 || lx >= SIZE_X ||
                        ly < 0 || ly >= SIZE_Y ||
                        lz < 0 || lz >= SIZE_Z)
                        continue;

                    // enlever les coins pour éviter cube parfait
                    if (std::abs(dx) == radius && std::abs(dz) == radius)
                    {
                            continue;
                    }

                    if (chunkData.blocks[Index(lx, ly, lz)] == BlockRegistry::AIR)
                        chunkData.blocks[Index(lx, ly, lz)] = leaves;
                }
            }
        }
    }
}

void LandGen::operator()(GeneratedChunk &chunk, GenerationData &data)
{
    std::unique_ptr<FastNoiseSIMD> noise = std::unique_ptr<FastNoiseSIMD>(FastNoiseSIMD::NewFastNoiseSIMD(data.seed));

    noise->SetNoiseType(FastNoiseSIMD::NoiseType::Simplex);
    noise->SetFrequency(TreeFrequency);

    float* NoiseSet = FastNoiseSIMD::GetEmptySet(SIZE_X);

    for (int z = 0; z < SIZE_Z; ++z)
    {
        noise->FillNoiseSet(NoiseSet, chunk.position.x, 0, chunk.position.y + z, SIZE_X, 1, 1);
        for (int x = 0; x < SIZE_X; ++x)
        {
            const int h = glm::floor(data.NoiseValue[Index(x, z)]);
            if (h >= 6)
            {
                for (int y = h - 3; y <= h; ++y)
                {
                    if (chunk.blocks[Index(x, y, z)] == BlockRegistry::STONE)
                    {
                        if (y == h - 1)
                            chunk.blocks[Index(x, y, z)] = BlockRegistry::GRASS;
                        else
                            chunk.blocks[Index(x, y, z)] = BlockRegistry::DIRT;
                    }
                }
            }
            if (h <= 5)
            {
                for (int y = -5; y < 5; ++y)
                {
                    if (chunk.blocks[Index(x, y, z)] == BlockRegistry::STONE)
                        chunk.blocks[Index(x, y, z)] = BlockRegistry::Get("sand");
                }
            }
            if (h >= 10)
            {
                if (NoiseSet[x] > TreeThreshhold)
                    PlaceTree(chunk, x, h, z);
            }
        }
    }
    FastNoiseSIMD::FreeNoiseSet(NoiseSet);
}
