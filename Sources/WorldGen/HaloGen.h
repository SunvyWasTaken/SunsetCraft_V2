//
// Created by sunvy on 16/06/2026.
//

#pragma once

#include "WorldGen.h"

struct HaloGen : public GenLayout
{
    HaloGen();
    ~HaloGen() override;
    void operator()(Chunk& chunk, ChunkData& data) override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
