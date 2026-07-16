//
// Created by sunvy on 21/06/2026.
//

#include "Sky.h"

#include <glm/ext/matrix_transform.hpp>

#include "Render/Resources/Drawable.h"
#include "Render/Resources/Material.h"
#include "Render/Core/RenderCommand.h"
#include "Render/Core/Shader.h"
#include "Render/BufferObject/Buffers.h"
#include "Render/BufferObject/VertexArray.h"
#include "Render/Meshes/Mesh.h"

namespace
{
    struct Vertex
    {
        glm::vec3 position;
    };

    std::vector<Vertex> GenerateSkyDome(float radius, int sectors, int stacks)
    {
        std::vector<Vertex> vertices;
        constexpr float PI = std::numbers::pi_v<float>;

        for (int i = 0; i <= stacks; ++i)  // vertical (stack)
        {
            float phi = PI * ((float)i / stacks);

            for (int j = 0; j <= sectors; ++j) // horizontal (sector)
            {
                float theta = 2.0f * PI * ((float)j / sectors); // 0 -> 2PI

                float x = radius * sinf(phi) * cosf(theta);
                float y = radius * cosf(phi);
                float z = radius * sinf(phi) * sinf(theta);

                vertices.push_back(Vertex{ glm::vec3(x, y, z) });
            }
        }

        return vertices;
    }

    std::vector<uint32_t> GenerateSkyDomeIndices(int sectors, int stacks)
    {
        std::vector<uint32_t> indices;

        for (int i = 0; i < stacks; ++i)
        {
            int k1 = i * (sectors + 1);
            int k2 = k1 + sectors + 1;

            for (int j = 0; j < sectors; ++j, ++k1, ++k2)
            {
                indices.push_back(k1);
                indices.push_back(k1 + 1);
                indices.push_back(k2);

                indices.push_back(k1 + 1);
                indices.push_back(k2 + 1);
                indices.push_back(k2);
            }
        }

        return indices;
    }

    constexpr float size = 5000.f;
    constexpr float height = 120.f;

    std::array<glm::vec3, 4> CloudVertices = {
        glm::vec3{-size, height, -size},
        glm::vec3{size, height, -size},
        glm::vec3{size, height, size},
        glm::vec3{-size, height, size}
    };
}

SkyCubed::SkyCubed()
    : m_Drawable(std::make_unique<Sunset::Drawable>())
{
    LOG("SunsetCraft", info, "SkyDome generated")
    {
        auto shader = std::make_shared<Sunset::Shader>(SHADERS_PATH "SkyCube.vert", SHADERS_PATH "SkyCube.frag");
        m_Drawable->m_Material->m_Shader = shader;

        std::vector<Vertex> vertices = GenerateSkyDome(500.f, 64, 32);
        std::vector<uint32_t> indices = GenerateSkyDomeIndices(64, 32);

        auto vbo = std::make_shared<Sunset::VertexBuffer>(vertices.data(), vertices.size(), sizeof(Vertex));
        vbo->SetLayout({Sunset::BufferElement{Sunset::ShaderDataType::Float3, "position"}});

        auto ebo = std::make_shared<Sunset::IndiceBuffer>(indices);

        auto vao = std::make_unique<Sunset::VertexArray>();
        vao->AddVertexBuffer(vbo);
        vao->AddIndexBuffer(*ebo);

        m_Drawable->m_Mesh = std::make_unique<Sunset::Mesh>(vao);
        m_Drawable->m_Mesh->m_VertexBuffer = vbo;
        m_Drawable->m_Mesh->m_IndiceBuffer = ebo;
    }
}

SkyCubed::~SkyCubed()
{
}

void SkyCubed::Update(const float deltaTime)
{
}

void SkyCubed::Draw() const
{
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    Sunset::RenderCommand::Submit(*m_Drawable, model);
}
