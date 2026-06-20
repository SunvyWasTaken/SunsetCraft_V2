//
// Created by sunvy on 20/06/2026.
//

#include "LandGen.h"

void LandGen::operator()(GeneratedChunk &chunk, GenerationData &data)
{
    for (int x = 0; x < SIZE_X; ++x)
    {
        for (int z = 0; z < SIZE_Z; ++z)
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
        }
    }
}
