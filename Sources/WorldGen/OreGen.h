//
// Created by sunvy on 21/06/2026.
//

#pragma once

#include "WorldGen.h"

struct OreGen : public GenLayout
{
    ~OreGen() override = default;
    void operator()(GeneratedChunk& chunk, GenerationData& data) override;
};
