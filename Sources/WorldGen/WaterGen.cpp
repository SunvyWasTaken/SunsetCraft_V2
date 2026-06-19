//
// Created by sunvy on 19/06/2026.
//

#include "WaterGen.h"

void WaterGen::operator()(GeneratedChunk &chunk, GenerationData &data)
{
    for (int x = 0; x < SIZE_X; ++x)
        for (int z = 0; z < SIZE_Z; ++z)
        {
            const int i1 = x + z * SIZE_X;
            for (int y = -SIZE_Y; y <= 0; ++y)
            {
                const int i2 = i1 + (y + SIZE_Y) * SIZE_X * SIZE_Z;
                if (chunk.blocks[i2] == BlockRegistry::AIR)
                    chunk.blocks[i2] = BlockRegistry::WATER;
            }
        }
}
