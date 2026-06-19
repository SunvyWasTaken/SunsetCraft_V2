//
// Created by sunvy on 19/06/2026.
//

#pragma once

#include "WorldGen.h"

struct WaterGen : public GenLayout
{
    ~WaterGen() override = default;
    void operator()(GeneratedChunk& chunk, GenerationData& data) override;
};
