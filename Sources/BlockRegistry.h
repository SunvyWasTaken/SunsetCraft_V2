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
    static void Destroy();
    static BlockId Get(const std::string& name);
    static bool IsTransparent(BlockId id);
    static BlockId AIR;
    static BlockId GRASS;
    static BlockId DIRT;
    static BlockId STONE;
    static BlockId WATER;
};