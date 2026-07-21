//
// Created by Codex on 21/07/2026.
//

#include "ShadowMap.h"

#include <glad/glad.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>

#include "ChunkRegistry.h"
#include "Core/Application.h"
#include "Core/WindowSetting.h"
#include "Render/Core/Shader.h"

namespace
{
    constexpr int ShadowMapSize = 2048;
    constexpr float ShadowDistance = 128.0f;
    constexpr float LightHeight = 180.0f;
}

ShadowMap::ShadowMap()
{
    m_Shader = std::make_unique<Sunset::Shader>(SHADERS_PATH "ShadowChunk.vert", SHADERS_PATH "ShadowDepth.frag");

    glGenFramebuffers(1, &m_FrameBuffer);
    glGenTextures(1, &m_DepthTexture);

    glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, ShadowMapSize, ShadowMapSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    constexpr float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    m_IsValid = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

ShadowMap::~ShadowMap()
{
    Release();
}

void ShadowMap::Render(const glm::vec3& center, const glm::vec3& sunDirection, ChunkRegistry& registry)
{
    if (!m_IsValid || sunDirection.y <= -0.08f)
        return;

    m_LightSpaceMatrix = BuildLightSpaceMatrix(center, sunDirection);

    glViewport(0, 0, ShadowMapSize, ShadowMapSize);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_FRONT);

    m_Shader->Use();
    m_Shader->SetMat4("u_LightSpaceMatrix", m_LightSpaceMatrix);
    registry.DrawShadowDepth(*m_Shader);

    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    const glm::ivec2 windowSize = Sunset::Application::GetSetting().WindowSize;
    glViewport(0, 0, windowSize.x, windowSize.y);
}

void ShadowMap::BindForRead(const int textureUnit) const
{
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
    glActiveTexture(GL_TEXTURE0);
}

ShadowRenderData ShadowMap::GetRenderData(const int textureUnit) const
{
    return ShadowRenderData{
        .enabled = m_IsValid,
        .depthTexture = m_DepthTexture,
        .textureUnit = textureUnit,
        .lightSpaceMatrix = m_LightSpaceMatrix
    };
}

glm::mat4 ShadowMap::BuildLightSpaceMatrix(const glm::vec3& center, const glm::vec3& sunDirection) const
{
    const glm::vec3 lightDir = glm::normalize(sunDirection);
    const glm::vec3 lightPosition = center + lightDir * LightHeight;
    const glm::vec3 up = std::abs(lightDir.y) > 0.95f ? glm::vec3{0.0f, 0.0f, 1.0f} : glm::vec3{0.0f, 1.0f, 0.0f};

    const glm::mat4 lightView = glm::lookAt(lightPosition, center, up);
    const glm::mat4 lightProjection = glm::ortho(-ShadowDistance, ShadowDistance, -ShadowDistance, ShadowDistance, 1.0f, LightHeight * 2.4f);

    return lightProjection * lightView;
}

void ShadowMap::Release()
{
    glDeleteFramebuffers(1, &m_FrameBuffer);
    glDeleteTextures(1, &m_DepthTexture);

    m_FrameBuffer = 0;
    m_DepthTexture = 0;
    m_IsValid = false;
}
