//
// Created by sunvy on 05/06/2026.
//

#include "MainMenu.h"

#include <random>
#include <imgui.h>

#include "Button.h"
#include "Game.h"
#include "Core/Application.h"
#include "Core/ApplicationSetting.h"
#include "Network/NetworkService.h"
#include "Render/Texture.h"
#include "Image.h"
#include "Panel.h"
#include "SunsetCraftInstance.h"
#include "Text.h"
#include "VerticalBox.h"

namespace
{
    std::ranlux24_base rng{std::random_device{}()};
    int seed = 0;
    std::unique_ptr<Sunset::Texture> m_Image = nullptr;
    std::vector<std::string> saves;
}

MainMenu::~MainMenu()
{
    m_Image.reset();
}

void MainMenu::Init()
{
    Layer::Init();

    if (Sunset::SaveSystem::Load(SAVE_PATH "GameSaved.bin", saves))
    {
        for (const auto& save : saves)
        {
            LOG("SunsetCraft", info, "Save {} found", save);
        }
    }
    else
    {
        if (Sunset::SaveSystem::Save(SAVE_PATH "GameSaved.bin", saves))
        {

        }
    }


    m_Image = std::make_unique<Sunset::Texture>();
    m_Image->LoadImage(RESOURCES "Logo/Logo.png");

    const auto& setting = Sunset::Application::GetSetting();

    constexpr glm::ivec2 LogoSize = {2172/3, 724/3};

    auto panel = SRmGUI::SNew<SRmGUI::Panel>()
        .Child(
            SRmGUI::SNew<SRmGUI::Image>()
            .Position((setting.WindowSize/2) - (LogoSize / 2) - glm::ivec2{0, 120})
            .Size(LogoSize)
            .Image(m_Image->GetId())
            )
        .Child(
            SRmGUI::SNew<SRmGUI::VerticalBox>()
            .Position((setting.WindowSize / 2) - glm::ivec2{75, -85})
            .Size({150, 150})
            .Child(
                SRmGUI::SNew<SRmGUI::Button>()
                .Padding({5.f, 5.f, 5.f, 5.f})
                .OnClicked([&]()
                {
                    Sunset::NetworkService::Init();
                    Sunset::NetworkService::Get().Host(7777, 2);

                    Sunset::SaveSystem::Save(SAVE_PATH "World1.bin", seed);

                    auto& app = Sunset::Application::GetApplication();
                    app.ClearLayer();
                    app.LoadLayer<GameLayer>("World1");
                })
                .Child(
                    SRmGUI::SNew<SRmGUI::Text>()
                    .Text("Play")
                    .Color({0.1f, 0.1f, 0.1f, 1.f})
                    .Padding({0.f, 0.f, 0.f, 40.f})
                )
            )
            .Child(
                SRmGUI::SNew<SRmGUI::Button>()
                .Padding({5.f, 5.f, 5.f, 5.f})
                .OnClicked([&]()
                {
                    Sunset::Application::CloseApplication();
                })
                .Child(
                    SRmGUI::SNew<SRmGUI::Text>()
                    .Text("Quit")
                    .Color({0.1f, 0.1f, 0.1f, 1.f})
                    .Padding({0.f, 0.f, 0.f, 40.f})
                )
            )
        );

    AddToViewport(panel.ToShared());
}

void MainMenu::OnUpdate(float dt)
{
    Layer::OnUpdate(dt);
}

void MainMenu::OnDraw()
{
    Layer::OnDraw();

    ImGui::Begin("Parameter");
    ImGui::InputInt("Seed", &seed);
    ImGui::SameLine();
    if (ImGui::Button("Random"))
    {
        std::uniform_int_distribution<int> dist(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
        seed = dist(rng);
    }
    static char worldName[50] = {};
    ImGui::InputText("World Name", worldName, 50);
    if (ImGui::Button("Create World"))
    {
        saves.emplace_back(worldName);
        if (Sunset::SaveSystem::Save(SAVE_PATH "GameSaved.bin", saves))
        {
            LOG("SunsetCraft", info, "Save {} Success", std::string(worldName));
            Sunset::SaveSystem::Save(SAVE_PATH + std::string(worldName), seed);
        }
    }
    for (const auto& save : saves)
    {
        if (ImGui::Button(save.c_str()))
        {
            Sunset::NetworkService::Init();
            Sunset::NetworkService::Get().Host(7777, 2);

            auto& app = Sunset::Application::GetApplication();
            app.ClearLayer();
            app.LoadLayer<GameLayer>(save);
        }
    }
    ImGui::End();
}
