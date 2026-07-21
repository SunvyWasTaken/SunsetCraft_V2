//
// Created by sunvy on 21/07/2026.
//

#pragma once

#include "WorldGen.h"

class RiverGen : public GenLayout
{
public:
    void operator()(GeneratedChunk& chunk, GenerationData& data);
};
