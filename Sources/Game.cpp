//
// Created by sunvy on 22/05/2026.
//

#include "Game.h"

#include <imgui.h>
#include <random>
#include <glm/gtc/type_ptr.hpp>

#include "FastNoiseSIMD.h"
#include "Render/Image.h"
#include "Render/Texture.h"

namespace
{
    int seed = 0;
    std::ranlux24_base rng{std::random_device{}()};
    int ImageSize = 100;
    bool IsDirty = false;

    struct Noises
    {
        int NoiseType = 0;
        int FractalOctaves = 1;
        float Frequency = 1.f;
        std::unique_ptr<SunsetEngine::Textures> texture = nullptr;
        std::vector<glm::fvec2> points;
    };

    float* NoiseValue = nullptr;

    std::unique_ptr<SunsetEngine::Textures> texture = nullptr;

    std::vector<Noises> noises;
    std::vector<std::string> NoiseName;
    int currentSelectNoise = 0;

    FastNoiseSIMD::NoiseType ItoNoise(const int i)
    {
        switch (i)
        {
            case 0:
                return FastNoiseSIMD::NoiseType::Value;
            case 1:
                return FastNoiseSIMD::NoiseType::ValueFractal;
            case 2:
                return FastNoiseSIMD::NoiseType::Perlin;
            case 3:
                return FastNoiseSIMD::NoiseType::PerlinFractal;
            case 4:
                return FastNoiseSIMD::NoiseType::Simplex;
            case 5:
                return FastNoiseSIMD::NoiseType::SimplexFractal;
            case 6:
                return FastNoiseSIMD::NoiseType::WhiteNoise;
            case 7:
                return FastNoiseSIMD::NoiseType::Cellular;
            case 8:
                return FastNoiseSIMD::NoiseType::Cubic;
            case 9:
                return FastNoiseSIMD::NoiseType::CubicFractal;
            default:
                return FastNoiseSIMD::NoiseType::Value;
        }
    }

    float GetNoiseValue(Noises& n, float noise)
    {
        if (n.points.empty())
            return 0.f;

        if (noise <= n.points.front().x)
            return n.points.front().x;

        if (noise >= n.points.back().x)
            return n.points.back().x;

        for (size_t i = 0; i < n.points.size(); ++i)
        {
            const glm::vec2 p1 = n.points[i];
            const glm::vec2 p2 = n.points[i + 1];

            if (noise >= p1.x && noise <= p2.x)
            {
                float t = (noise - p1.x) / (p2.x - p1.x);
                return p1.y + t * (p2.y - p1.y);
            }
        }
        return 0.f;
    }
}

GameLayer::GameLayer()
{
    NoiseValue = FastNoiseSIMD::GetEmptySet(50*50);
}

GameLayer::~GameLayer()
{
    texture.reset();
    FastNoiseSIMD::FreeNoiseSet(NoiseValue);
}

void GameLayer::OnUpdate(float dt)
{
    if (!IsDirty)
        return;

    if (currentSelectNoise >= noises.size() || currentSelectNoise < 0)
        currentSelectNoise = 0;

    Noises& n = noises[currentSelectNoise];

    std::unique_ptr<FastNoiseSIMD> noise = std::unique_ptr<FastNoiseSIMD>(FastNoiseSIMD::NewFastNoiseSIMD(seed));
    noise->SetNoiseType(ItoNoise(n.NoiseType));
    noise->SetFractalOctaves(n.FractalOctaves);
    noise->SetFrequency(n.Frequency);

    noise->FillNoiseSet(NoiseValue, 0, 0, 0, 50, 50, 1);

    unsigned char* data = new unsigned char[50 * 50 * 4];

    for (int i = 0; i < 50 * 50; ++i)
    {
        NoiseValue[i] = (NoiseValue[i] + 1) / 2;
        unsigned char value = static_cast<unsigned char>(GetNoiseValue(n, NoiseValue[i]) * 255.0f);
        data[i * 4] = value;
        data[i * 4 + 1] = value;
        data[i * 4 + 2] = value;
        data[i * 4 + 3] = 255;
    }

    std::vector<SunsetEngine::Image> imgs;
    imgs.emplace_back();
    imgs.at(0).SetData(data);
    imgs.at(0).width = 50;
    imgs.at(0).height = 50;
    imgs.at(0).nbrChannels = 4;

    std::unique_ptr tex = std::make_unique<SunsetEngine::Textures>("noise", imgs, 50, 50);

    n.texture.reset(tex.release());

    IsDirty = false;
}

void GameLayer::OnDraw()
{
    ImGui::Begin("Parameter");
    ImGui::InputInt("Seed", &seed);
    ImGui::SameLine();
    if (ImGui::Button("Random"))
    {
        std::uniform_int_distribution<int> dist(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
        seed = dist(rng);
    }

    ImGui::InputInt("Image Size", &ImageSize);

    if (ImGui::Button("delete"))
    {}
    ImGui::SameLine();
    static char name[128] = "name?";
    if (ImGui::Button("Add"))
    {
        NoiseName.emplace_back(name);
        noises.emplace_back();
    }
    ImGui::InputText("Noise Name", name, 128);

    if (!noises.empty())
    {
        if (currentSelectNoise >= noises.size() || currentSelectNoise < 0)
            currentSelectNoise = 0;

        if (ImGui::BeginCombo("Noises", NoiseName[currentSelectNoise].c_str()))
        {
            for (int i = 0; i < NoiseName.size(); ++i)
            {
                bool isSelected = (currentSelectNoise == i);

                if (ImGui::Selectable(NoiseName[i].c_str(), isSelected))
                {
                    currentSelectNoise = i;
                }

                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }

        Noises& n = noises[currentSelectNoise];

        const char* items[]{ "Value", "ValueFractal", "Perlin", "PerlinFractal", "Simplex", "SimplexFractal", "WhiteNoise", "Cellular", "Cubic", "CubicFractal" };
        ImGui::Combo("Noise Type", &n.NoiseType, items, 10);
        ImGui::InputInt("Fractal Octave", &n.FractalOctaves);
        ImGui::SliderFloat("Frequency", &n.Frequency, 0.001f, 2);
        if (ImGui::Button("Add points"))
        {
            n.points.emplace_back();
        }
        for (int i = 0; i < n.points.size(); ++i)
        {
            std::string pointName = "##Point " + std::to_string(i);
            ImGui::DragFloat2(pointName.c_str(), glm::value_ptr(n.points[i]), 0.1, -1, 1);
            ImGui::SameLine();
            std::function<void()> action;
            if (ImGui::Button(("-##" + pointName).c_str()))
            {
                action = [&](){ n.points.erase(n.points.begin() + i); };
            }
            if (action)
                action();
        }
        if(ImGui::Button("Generate"))
        {
            IsDirty = true;
        }
    }

    ImGui::End();
    ImGui::Begin("Viewport");
    for (auto& n : noises)
    {
        if (n.texture)
            ImGui::Image(n.texture->operator()(), {(float)ImageSize, (float)ImageSize});
    }
    ImGui::End();
}
