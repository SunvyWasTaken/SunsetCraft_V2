//
// Created by sunvy on 17/07/2026.
//

#include "TreeGen.h"

namespace
{
    constexpr int TreeMinHeight = 10;
    constexpr int TrunkHeight = 4;

    struct TreeSettings
    {
        // 0.0f = aucun arbre, 1.0f = chaque cellule d'arbre valide en reçoit un.
        float density = 0.35f;
        // Taille d'une cellule d'arbre en blocs. Plus la valeur est grande, plus les arbres sont espacés.
        int spacing = 5;
    } TreeSettingsInstance;

    int FloorDiv(const int value, const int divisor)
    {
        int quotient = value / divisor;
        const int remainder = value % divisor;
        if (remainder != 0 && ((remainder < 0) != (divisor < 0)))
            --quotient;

        return quotient;
    }

    uint32_t Hash(uint32_t value)
    {
        value ^= value >> 16;
        value *= 0x7feb352du;
        value ^= value >> 15;
        value *= 0x846ca68bu;
        value ^= value >> 16;
        return value;
    }

    uint32_t HashCell(const int cellX, const int cellZ, const int seed, const uint32_t salt = 0u)
    {
        uint32_t value = static_cast<uint32_t>(cellX) * 0x9e3779b9u;
        value ^= static_cast<uint32_t>(cellZ) * 0x85ebca6bu;
        value ^= static_cast<uint32_t>(seed) * 0xc2b2ae35u;
        value ^= salt;
        return Hash(value);
    }

    float Hash01(const uint32_t value)
    {
        return static_cast<float>(value) / static_cast<float>(UINT32_MAX);
    }

    bool IsTreeCandidate(const int worldX, const int worldZ, const int seed, const TreeSettings& settings)
    {
        const int spacing = std::max(1, settings.spacing);
        const float density = std::clamp(settings.density, 0.0f, 1.0f);

        if (density <= 0.0f)
            return false;

        const int cellX = FloorDiv(worldX, spacing);
        const int cellZ = FloorDiv(worldZ, spacing);

        if (Hash01(HashCell(cellX, cellZ, seed)) > density)
            return false;

        const int baseX = cellX * spacing;
        const int baseZ = cellZ * spacing;
        const int treeX = baseX + static_cast<int>(HashCell(cellX, cellZ, seed, 0x68bc21ebu) % static_cast<uint32_t>(spacing));
        const int treeZ = baseZ + static_cast<int>(HashCell(cellX, cellZ, seed, 0x02e5be93u) % static_cast<uint32_t>(spacing));

        return worldX == treeX && worldZ == treeZ;
    }

    void PlaceTree(GeneratedChunk& chunk, const int x, const int y, const int z)
    {
        const BlockId wood = BlockRegistry::Get("wood");
        const BlockId leaves = BlockRegistry::Get("oak_leaf");

        if (wood == BlockRegistry::AIR || leaves == BlockRegistry::AIR)
            return;

        for (int i = 0; i <= TrunkHeight; ++i)
        {
            const int localY = y + i;
            if (localY >= SIZE_Y)
                break;

            chunk.blocks[Index(x, localY, z)] = i == TrunkHeight ? leaves : wood;
        }

        const int topY = y + TrunkHeight;
        for (int layer = 0; layer < 4; ++layer)
        {
            const int currentY = topY - layer;
            const int radius = (layer == 0 || layer == 3) ? 1 : 2;

            if (currentY < -SIZE_Y || currentY >= SIZE_Y)
                continue;

            for (int dz = -radius; dz <= radius; ++dz)
            {
                for (int dx = -radius; dx <= radius; ++dx)
                {
                    const int lx = x + dx;
                    const int lz = z + dz;

                    if (lx < 0 || lx >= SIZE_X || lz < 0 || lz >= SIZE_Z)
                        continue;

                    if (std::abs(dx) == radius && std::abs(dz) == radius)
                        continue;

                    BlockId& block = chunk.blocks[Index(lx, currentY, lz)];
                    if (block == BlockRegistry::AIR)
                        block = leaves;
                }
            }
        }
    }
}

void TreeGen::operator()(GeneratedChunk& chunk, GenerationData& data)
{
    for (int z = 0; z < SIZE_Z; ++z)
    {
        for (int x = 0; x < SIZE_X; ++x)
        {
            const int h = static_cast<int>(std::floor(data.NoiseValue[Index(x, z)]));
            if (h < TreeMinHeight || h + TrunkHeight >= SIZE_Y)
                continue;

            if (chunk.blocks[Index(x, h - 1, z)] != BlockRegistry::GRASS)
                continue;

            const int worldX = chunk.position.x * SIZE_X + x;
            const int worldZ = chunk.position.y * SIZE_Z + z;
            if (IsTreeCandidate(worldX, worldZ, data.seed, TreeSettingsInstance))
                PlaceTree(chunk, x, h, z);
        }
    }
}
