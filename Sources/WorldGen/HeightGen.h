//
// Created by sunvy on 15/06/2026.
//

#pragma once

#include "WorldGen.h"

struct HeightGen : public GenLayout
{
    ~HeightGen() override = default;
    void operator()(Chunk& chunk, ChunkData& data) override;
};
