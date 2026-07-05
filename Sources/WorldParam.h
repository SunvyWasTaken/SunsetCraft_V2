//
// Created by sunvy on 05/07/2026.
//

#pragma once

#include "SaveSystem/BinaryArchive.h"

struct WorldParam
{
    std::string Name{};
    int seed = 0;
};


template <typename Archive>
void Serialize(Archive& ar, WorldParam& param)
{
    ar(param.Name);
    ar(param.seed);
}

