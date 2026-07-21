//
// Created by Codex on 21/07/2026.
//

#pragma once

#include <cstdint>
#include <memory>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace Sunset
{
    class Shader;
}

class ChunkRegistry;

struct ShadowRenderData
{
    bool enabled = false;
    std::uint32_t depthTexture = 0;
    int textureUnit = 1;
    glm::mat4 lightSpaceMatrix{1.0f};
};

class ShadowMap
{
public:
    ShadowMap();
    ~ShadowMap();

    ShadowMap(const ShadowMap&) = delete;
    ShadowMap& operator=(const ShadowMap&) = delete;

    void Render(const glm::vec3& center, const glm::vec3& sunDirection, ChunkRegistry& registry);
    void BindForRead(int textureUnit) const;

    [[nodiscard]] ShadowRenderData GetRenderData(int textureUnit = 1) const;

private:
    glm::mat4 BuildLightSpaceMatrix(const glm::vec3& center, const glm::vec3& sunDirection) const;
    void Release();

    std::uint32_t m_FrameBuffer = 0;
    std::uint32_t m_DepthTexture = 0;
    std::unique_ptr<Sunset::Shader> m_Shader;
    glm::mat4 m_LightSpaceMatrix{1.0f};
    bool m_IsValid = false;
};
