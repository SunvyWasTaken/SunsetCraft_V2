//
// Created by sunvy on 16/06/2026.
//

#pragma once

#include "WorldGen.h"

struct CaveGen : public GenLayout
{
    ~CaveGen() override = default;
    void operator()(GeneratedChunk& chunk, GenerationData& data) override;
};
