//
// Created by sunvy on 21/07/2026.
//

#include "RiverGen.h"

#include "../Noise.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <unordered_map>
#include <vector>

namespace
{
    constexpr int OceanLevel = 0;
    constexpr int RegionSize = 512;
    constexpr int RegionSearchRadius = 1;
    constexpr int SourcesPerRegion = 1;
    constexpr int SourceCandidates = 8;
    constexpr int MinSourceHeight = 26;
    constexpr int MaxRiverSteps = 48;
    constexpr float StepLength = 10.0f;
    constexpr float MinDropPerStep = 0.18f;
    constexpr float MaxTerrainCut = 10.0f;
    constexpr float BaseRiverWidth = 4.2f;
    constexpr float BankWidth = 7.0f;
    constexpr int ChannelDepth = 4;
    constexpr int MinBankDrop = 3;

    struct RiverNode
    {
        glm::vec2 position{0.0f};
        float waterLevel = 0.0f;
        float width = BaseRiverWidth;
    };

    struct RiverPath
    {
        std::vector<RiverNode> nodes;
    };

    struct RiverHit
    {
        float distance = std::numeric_limits<float>::max();
        float waterLevel = 0.0f;
        float width = 0.0f;
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

    uint32_t HashCell(const int x, const int z, const int seed, const uint32_t salt)
    {
        uint32_t value = static_cast<uint32_t>(x) * 0x9e3779b9u;
        value ^= static_cast<uint32_t>(z) * 0x85ebca6bu;
        value ^= static_cast<uint32_t>(seed) * 0xc2b2ae35u;
        value ^= salt;
        return Hash(value);
    }

    float Hash01(const uint32_t value)
    {
        return static_cast<float>(value) / static_cast<float>(UINT32_MAX);
    }

    float SmoothStep(const float edge0, const float edge1, const float value)
    {
        const float t = std::clamp((value - edge0) / (edge1 - edge0), 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    }

    float LengthSquared(const glm::vec2 value)
    {
        return value.x * value.x + value.y * value.y;
    }

    glm::vec2 SafeNormalize(const glm::vec2 value, const glm::vec2 fallback)
    {
        const float length = std::sqrt(LengthSquared(value));
        if (length <= 0.0001f)
            return fallback;

        return value / length;
    }

    int ChunkKey(const int chunkX, const int chunkZ)
    {
        return (chunkX * 73856093) ^ (chunkZ * 19349663);
    }

    struct HeightSampler
    {
        float Get(const int worldX, const int worldZ)
        {
            const int chunkX = FloorDiv(worldX, SIZE_X);
            const int chunkZ = FloorDiv(worldZ, SIZE_Z);
            const int localX = worldX - chunkX * SIZE_X;
            const int localZ = worldZ - chunkZ * SIZE_Z;
            const int key = ChunkKey(chunkX, chunkZ);

            auto it = cache.find(key);
            if (it == cache.end())
            {
                std::vector<float> values;
                Noise::Get(values, glm::ivec2{chunkX * SIZE_X, chunkZ * SIZE_Z});
                it = cache.emplace(key, std::move(values)).first;
            }

            return it->second[Index(localX, localZ)];
        }

        std::unordered_map<int, std::vector<float>> cache;
    };

    glm::vec2 PickSource(const int regionX, const int regionZ, const int seed, const int sourceIndex, HeightSampler& sampler)
    {
        const int baseX = regionX * RegionSize;
        const int baseZ = regionZ * RegionSize;
        glm::vec2 bestPosition{static_cast<float>(baseX + RegionSize / 2), static_cast<float>(baseZ + RegionSize / 2)};
        float bestHeight = -std::numeric_limits<float>::max();

        for (int i = 0; i < SourceCandidates; ++i)
        {
            const uint32_t salt = 0x4489af31u + static_cast<uint32_t>(sourceIndex * 97 + i * 13);
            const int x = baseX + static_cast<int>(HashCell(regionX, regionZ, seed, salt) % RegionSize);
            const int z = baseZ + static_cast<int>(HashCell(regionX, regionZ, seed, salt ^ 0xa24f9c7bu) % RegionSize);
            const float height = sampler.Get(x, z);

            if (height > bestHeight)
            {
                bestHeight = height;
                bestPosition = glm::vec2{static_cast<float>(x), static_cast<float>(z)};
            }
        }

        return bestPosition;
    }

    RiverPath BuildRiverPath(const int regionX, const int regionZ, const int seed, const int sourceIndex, HeightSampler& sampler)
    {
        RiverPath path;
        const glm::vec2 source = PickSource(regionX, regionZ, seed, sourceIndex, sampler);
        const float sourceTerrainHeight = sampler.Get(static_cast<int>(std::floor(source.x)), static_cast<int>(std::floor(source.y)));
        if (sourceTerrainHeight < MinSourceHeight)
            return path;

        glm::vec2 position = source;
        glm::vec2 direction = SafeNormalize(glm::vec2{
            Hash01(HashCell(regionX, regionZ, seed, 0x7b261391u + sourceIndex)) - 0.5f,
            Hash01(HashCell(regionX, regionZ, seed, 0x9cd251e3u + sourceIndex)) - 0.5f
        }, glm::vec2{1.0f, 0.0f});

        float waterLevel = std::max(static_cast<float>(OceanLevel + 1), sourceTerrainHeight - MinBankDrop);
        path.nodes.push_back({position, waterLevel, BaseRiverWidth});

        for (int step = 0; step < MaxRiverSteps; ++step)
        {
            if (waterLevel <= OceanLevel + 0.35f)
                break;

            float bestScore = std::numeric_limits<float>::max();
            glm::vec2 bestPosition = position + direction * StepLength;
            float bestTerrainHeight = sampler.Get(static_cast<int>(std::floor(bestPosition.x)), static_cast<int>(std::floor(bestPosition.y)));

            for (int candidate = 0; candidate < 11; ++candidate)
            {
                const float angle = (-0.95f + static_cast<float>(candidate) * 0.19f) * 3.1415926f;
                const float cs = std::cos(angle);
                const float sn = std::sin(angle);
                const glm::vec2 candidateDirection{
                    direction.x * cs - direction.y * sn,
                    direction.x * sn + direction.y * cs
                };
                const glm::vec2 candidatePosition = position + candidateDirection * StepLength;
                const int candidateX = static_cast<int>(std::floor(candidatePosition.x));
                const int candidateZ = static_cast<int>(std::floor(candidatePosition.y));
                const float terrainHeight = sampler.Get(candidateX, candidateZ);
                const float cutPenalty = std::max(0.0f, terrainHeight - waterLevel - MaxTerrainCut) * 18.0f;
                const float turnPenalty = (1.0f - std::clamp(direction.x * candidateDirection.x + direction.y * candidateDirection.y, -1.0f, 1.0f)) * 9.0f;
                const float noise = Hash01(HashCell(candidateX, candidateZ, seed, 0x63b8a9edu)) * 4.0f;
                const float oceanPull = std::abs(terrainHeight - static_cast<float>(OceanLevel)) * 0.18f;
                const float score = terrainHeight * 1.35f + cutPenalty + turnPenalty + noise + oceanPull;

                if (score < bestScore)
                {
                    bestScore = score;
                    bestPosition = candidatePosition;
                    bestTerrainHeight = terrainHeight;
                }
            }

            const float nextWaterLevel = std::min(waterLevel - MinDropPerStep, bestTerrainHeight - MinBankDrop);
            waterLevel = std::max(static_cast<float>(OceanLevel), nextWaterLevel);
            direction = SafeNormalize(bestPosition - position, direction);
            position = bestPosition;

            const float lakeBonus = std::clamp(waterLevel - bestTerrainHeight, 0.0f, 22.0f);
            const float width = BaseRiverWidth + lakeBonus * 1.2f;
            path.nodes.push_back({position, waterLevel, width});

            if (bestTerrainHeight <= OceanLevel + 1.0f)
                break;
        }

        if (path.nodes.size() < 6 || path.nodes.back().waterLevel > OceanLevel + 8.0f)
            path.nodes.clear();

        return path;
    }

    RiverHit DistanceToSegment(const RiverNode& a, const RiverNode& b, const glm::vec2 point)
    {
        const glm::vec2 segment = b.position - a.position;
        const float segmentLength = LengthSquared(segment);
        if (segmentLength <= 0.0001f)
            return {};

        const float t = std::clamp(((point.x - a.position.x) * segment.x + (point.y - a.position.y) * segment.y) / segmentLength, 0.0f, 1.0f);
        const glm::vec2 closest = a.position + segment * t;
        const float width = a.width + (b.width - a.width) * t;
        const float waterLevel = a.waterLevel + (b.waterLevel - a.waterLevel) * t;
        const float distance = std::sqrt(LengthSquared(point - closest)) - width;
        return {distance, waterLevel, width};
    }

    RiverHit ClosestRiver(const int worldX, const int worldZ, const std::vector<RiverPath>& paths)
    {
        const glm::vec2 point{static_cast<float>(worldX), static_cast<float>(worldZ)};

        RiverHit bestHit;
        for (const RiverPath& path : paths)
        {
            for (size_t i = 1; i < path.nodes.size(); ++i)
            {
                const RiverHit hit = DistanceToSegment(path.nodes[i - 1], path.nodes[i], point);
                if (hit.distance < bestHit.distance)
                    bestHit = hit;
            }
        }

        return bestHit;
    }

    bool IsInsideWorldHeight(const int y)
    {
        return y >= -SIZE_Y && y < SIZE_Y;
    }

    void SetIfInside(GeneratedChunk& chunk, const int x, const int y, const int z, const BlockId block)
    {
        if (IsInsideWorldHeight(y))
            chunk.blocks[Index(x, y, z)] = block;
    }
}

void RiverGen::operator()(GeneratedChunk& chunk, GenerationData& data)
{
    SS_PROFILE_FUNCTION();

    HeightSampler sampler;
    std::vector<RiverPath> paths;
    const BlockId sand = BlockRegistry::Get("sand");
    const int chunkWorldX = chunk.position.x * SIZE_X;
    const int chunkWorldZ = chunk.position.y * SIZE_Z;
    const int regionX = FloorDiv(chunkWorldX, RegionSize);
    const int regionZ = FloorDiv(chunkWorldZ, RegionSize);

    for (int rz = regionZ - RegionSearchRadius; rz <= regionZ + RegionSearchRadius; ++rz)
    {
        for (int rx = regionX - RegionSearchRadius; rx <= regionX + RegionSearchRadius; ++rx)
        {
            for (int sourceIndex = 0; sourceIndex < SourcesPerRegion; ++sourceIndex)
            {
                RiverPath path = BuildRiverPath(rx, rz, data.seed, sourceIndex, sampler);
                if (!path.nodes.empty())
                    paths.emplace_back(std::move(path));
            }
        }
    }

    for (int z = 0; z < SIZE_Z; ++z)
    {
        for (int x = 0; x < SIZE_X; ++x)
        {
            const int worldX = chunk.position.x * SIZE_X + x;
            const int worldZ = chunk.position.y * SIZE_Z + z;
            const RiverHit river = ClosestRiver(worldX, worldZ, paths);

            if (river.distance >= BankWidth || river.waterLevel <= OceanLevel)
                continue;

            const size_t heightIndex = Index(x, z);
            const int originalHeight = static_cast<int>(std::floor(data.NoiseValue[heightIndex]));
            if (originalHeight <= OceanLevel + 1)
                continue;

            const int waterTop = static_cast<int>(std::floor(river.waterLevel));
            const bool inWater = river.distance <= 0.0f || originalHeight < waterTop;
            const float bankT = SmoothStep(0.0f, BankWidth, std::max(river.distance, 0.0f));
            const int bedHeight = waterTop - ChannelDepth;
            const int bankHeight = waterTop + MinBankDrop + static_cast<int>(std::round(bankT * 4.0f));
            const int targetHeight = inWater ? bedHeight : std::min(originalHeight, bankHeight);
            const int riverHeight = std::clamp(targetHeight, -SIZE_Y + 2, SIZE_Y - 2);

            for (int y = riverHeight; y < originalHeight && y < SIZE_Y; ++y)
                SetIfInside(chunk, x, y, z, BlockRegistry::AIR);

            if (inWater)
            {
                const int waterBottom = std::min(riverHeight, originalHeight);
                for (int y = waterBottom; y <= waterTop; ++y)
                    SetIfInside(chunk, x, y, z, BlockRegistry::WATER);

                SetIfInside(chunk, x, waterBottom - 1, z, sand);
                SetIfInside(chunk, x, waterBottom - 2, z, sand);
            }
            else
            {
                const BlockId bankBlock = originalHeight > 22 ? BlockRegistry::STONE : BlockRegistry::DIRT;
                SetIfInside(chunk, x, riverHeight - 1, z, bankBlock);
            }

            data.NoiseValue[heightIndex] = static_cast<float>(riverHeight);
        }
    }
}
