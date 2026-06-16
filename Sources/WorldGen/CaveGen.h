//
// Created by sunvy on 16/06/2026.
//

#pragma once

#include "WorldGen.h"

struct CaveGen : public GenLayout
{
    CaveGen();
    ~CaveGen() override;
    void operator()(Chunk& chunk, GenerationData& data) override;
private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
