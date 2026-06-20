//
// Created by sunvy on 20/06/2026.
//

#pragma once

#include "WorldGen.h"

struct LandGen : public GenLayout
{
    ~LandGen() override = default;
    void operator()(GeneratedChunk& chunk, GenerationData& data) override;
};
