//
// Created by sunvy on 31/05/2026.
//

#pragma once

using Block = uint8_t;

struct BlockRegistry
{
    static Block Get(const std::string& name);
    static Block AIR;
    static Block STONE;
};