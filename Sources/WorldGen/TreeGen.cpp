//
// Created by sunvy on 17/07/2026.
//

#include "TreeGen.h"

#include "HeightGen.h"
#include "LandGen.h"
#include "WaterGen.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace
{
    constexpr int TreeMinHeight = 10;
    constexpr int TrunkHeight = 4;
    constexpr int LeafRadius = 2;

    struct TreeSettings
    {
        // 0.0f = aucun arbre, 1.0f = chaque cellule d'arbre valide en reçoit un.
        float density = 0.35f;
        // Taille d'une cellule d'arbre en blocs. Plus la valeur est grande, plus les arbres sont espacés.
        int spacing = 5;
    } TreeSettingsInstance;

    struct PreTreeSource
    {
        explicit PreTreeSource(const glm::ivec2 sourcePosition, const int seed)
            : chunk(sourcePosition)
        {
            data.seed = seed;

            HeightGen{}(chunk, data);
            LandGen{}(chunk, data);
            WaterGen{}(chunk, data);
        }

        GeneratedChunk chunk;
        GenerationData data;
    };

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

    int WorldToChunkCoord(const int value, const int chunkSize)
    {
        return FloorDiv(value, chunkSize);
    }

    int WorldToLocalCoord(const int value, const int chunkCoord, const int chunkSize)
    {
        return value - chunkCoord * chunkSize;
    }

    int GetTreeBaseY(
        const int worldX,
        const int worldZ,
        const glm::ivec2 position,
        const GeneratedChunk& currentChunk,
        const GenerationData& currentData,
        std::vector<PreTreeSource>& sourceChunks,
        const int seed)
    {
        const int sourceX = WorldToLocalCoord(worldX, position.x, SIZE_X);
        const int sourceZ = WorldToLocalCoord(worldZ, position.y, SIZE_Z);
        auto baseYOrAir = [sourceX, sourceZ](const GeneratedChunk& sourceChunk, const GenerationData& sourceData)
        {
            const int h = static_cast<int>(std::floor(sourceData.NoiseValue[Index(sourceX, sourceZ)]));
            if (h <= -SIZE_Y || h > SIZE_Y)
                return -SIZE_Y;

            return sourceChunk.blocks[Index(sourceX, h - 1, sourceZ)] == BlockRegistry::GRASS ? h : -SIZE_Y;
        };

        if (position == currentChunk.position)
            return baseYOrAir(currentChunk, currentData);

        for (const PreTreeSource& source : sourceChunks)
        {
            if (source.chunk.position == position)
                return baseYOrAir(source.chunk, source.data);
        }

        sourceChunks.emplace_back(position, seed);
        const PreTreeSource& source = sourceChunks.back();
        return baseYOrAir(source.chunk, source.data);
    }

    void SetBlockIfInside(GeneratedChunk& chunk, const int worldX, const int y, const int worldZ, const BlockId block)
    {
        const int lx = worldX - chunk.position.x * SIZE_X;
        const int lz = worldZ - chunk.position.y * SIZE_Z;

        if (lx < 0 || lx >= SIZE_X || y < -SIZE_Y || y >= SIZE_Y || lz < 0 || lz >= SIZE_Z)
            return;

        chunk.blocks[Index(lx, y, lz)] = block;
    }

    BlockId& BlockAtWorld(GeneratedChunk& chunk, const int worldX, const int y, const int worldZ)
    {
        const int lx = worldX - chunk.position.x * SIZE_X;
        const int lz = worldZ - chunk.position.y * SIZE_Z;
        return chunk.blocks[Index(lx, y, lz)];
    }

    void PlaceTree(GeneratedChunk& chunk, const int worldX, const int y, const int worldZ)
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

            SetBlockIfInside(chunk, worldX, localY, worldZ, i == TrunkHeight ? leaves : wood);
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
                    if (std::abs(dx) == radius && std::abs(dz) == radius)
                        continue;

                    const int leafWorldX = worldX + dx;
                    const int leafWorldZ = worldZ + dz;
                    const int leafLocalX = leafWorldX - chunk.position.x * SIZE_X;
                    const int leafLocalZ = leafWorldZ - chunk.position.y * SIZE_Z;
                    if (leafLocalX < 0 || leafLocalX >= SIZE_X || leafLocalZ < 0 || leafLocalZ >= SIZE_Z)
                        continue;

                    BlockId& block = BlockAtWorld(chunk, leafWorldX, currentY, leafWorldZ);
                    if (block == BlockRegistry::AIR)
                        block = leaves;
                }
            }
        }
    }
}

void TreeGen::operator()(GeneratedChunk& chunk, GenerationData& data)
{
    std::vector<PreTreeSource> sourceChunks;

    const int minWorldX = chunk.position.x * SIZE_X - LeafRadius;
    const int maxWorldX = chunk.position.x * SIZE_X + SIZE_X + LeafRadius - 1;
    const int minWorldZ = chunk.position.y * SIZE_Z - LeafRadius;
    const int maxWorldZ = chunk.position.y * SIZE_Z + SIZE_Z + LeafRadius - 1;

    for (int worldZ = minWorldZ; worldZ <= maxWorldZ; ++worldZ)
    {
        for (int worldX = minWorldX; worldX <= maxWorldX; ++worldX)
        {
            if (!IsTreeCandidate(worldX, worldZ, data.seed, TreeSettingsInstance))
                continue;

            const glm::ivec2 sourcePosition{
                WorldToChunkCoord(worldX, SIZE_X),
                WorldToChunkCoord(worldZ, SIZE_Z)
            };
            const int h = GetTreeBaseY(worldX, worldZ, sourcePosition, chunk, data, sourceChunks, data.seed);
            if (h < TreeMinHeight || h + TrunkHeight >= SIZE_Y)
                continue;

            PlaceTree(chunk, worldX, h, worldZ);
        }
    }
}
