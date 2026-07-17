//
// Created by sunvy on 17/07/2026.
//

#pragma once

#include "WorldGen.h"

class TreeGen : public GenLayout
{
public:
    ~TreeGen() override = default;
    void operator()(GeneratedChunk& chunk, GenerationData& data) override;
};
