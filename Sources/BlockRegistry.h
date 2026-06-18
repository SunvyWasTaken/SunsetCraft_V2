//
// Created by sunvy on 31/05/2026.
//

#pragma once

using BlockId = uint8_t;

struct BlockType
{
    std::string name;
    BlockId id;
};

struct BlockRegistry
{
    static void Init();
    static BlockId Get(const std::string& name);
    static BlockId AIR;
    static BlockId STONE;
};