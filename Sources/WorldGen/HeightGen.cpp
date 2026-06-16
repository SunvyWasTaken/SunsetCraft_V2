//
// Created by sunvy on 15/06/2026.
//

#include "HeightGen.h"

#include "../Noise.h"
#include "../Chunk.h"

void HeightGen::operator()(Chunk &chunk, GenerationData &chunkData)
{
    SS_PROFILE_FUNCTION();
    std::vector<float> NoiseValue;
    Noise::Get(NoiseValue, chunk.m_Position * glm::ivec2{SIZE_X, SIZE_Z});
    for (int x = 0; x < SIZE_X; ++x)
        for (int z = 0; z < SIZE_Z; ++z)
        {
            const int i1 = x + z * SIZE_X;
            float h = NoiseValue[i1];
            for (int y = -SIZE_Y; y < SIZE_Y; ++y)
            {
                const int i2 = i1 + (y + SIZE_Y) * SIZE_X * SIZE_Z;
                if (y < floor(h))
                {
                    chunk.m_Blocks[i2] = BlockRegistry::STONE;
                }
            }
        }
}
