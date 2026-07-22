//
// Created by sunvy on 05/06/2026.
//

#include "MainMenu.h"

#include <random>

#include "Button.h"
#include "Game.h"
#include "Core/Application.h"
#include "Core/WindowSetting.h"
#include "Network/NetworkService.h"
#include "Render/Resources/Texture.h"
#include "Image.h"
#include "Panel.h"
#include "Text.h"
#include "VerticalBox.h"
#include "WidgetSwitch.h"
#include "WorldParam.h"
#include "SaveSystem/SaveSystem.h"

namespace
{
    std::ranlux24_base rng{std::random_device{}()};
    std::unique_ptr<Sunset::Texture> m_Image = nullptr;
    std::vector<WorldParam> saves;
    std::shared_ptr<SRmGUI::Text> m_Text = nullptr;
    WorldParam worldParam {};
    std::shared_ptr<SRmGUI::WidgetSwitch> m_WidgetSwitch = nullptr;
    std::shared_ptr<SRmGUI::VerticalBox> m_WorldButtons = nullptr;
}

MainMenu::~MainMenu()
{
    m_Text.reset();
    m_Image.reset();
}

void MainMenu::Init()
{
    Layer::Init();

    Sunset::SaveSystem::Load(SAVE_PATH "GameSaved.bin", saves);
    if (!saves.empty())
        worldParam = saves[0];

    m_Image = std::make_unique<Sunset::Texture>();
    m_Image->LoadImage(RESOURCES "Logo/Logo.png");

    const auto& setting = Sunset::Application::GetSetting();

    constexpr glm::ivec2 LogoSize = {2172/3, 724/3};

    auto panel = SRmGUI::SNew<SRmGUI::Panel>()
        .Child(
            SRmGUI::SNew<SRmGUI::Image>()
            .Position((setting.WindowSize/2) - (LogoSize / 2) - glm::ivec2{0, 120})
            .Size(LogoSize)
            .Image(m_Image->GetRendererId())
            )
        .Child(
            SRmGUI::SNew<SRmGUI::VerticalBox>()
            .Position((setting.WindowSize / 2) - glm::ivec2{75, -85})
            .Size({150, 350})
            .Child(
                SRmGUI::SNewAssign<SRmGUI::Text>(m_Text)
                .Text(worldParam.Name)
                .Color({1.f, 1.f, 1.f, 1.f})
                )
            .Child(
                SRmGUI::SNew<SRmGUI::Button>()
                .Padding({5.f, 5.f, 5.f, 5.f})
                .OnClicked([&]()
                {
                    // if (!saves.empty())
                    // {
                    //     m_WidgetSwitch->SetActive(1);
                    //     return;
                    // }

                    Sunset::NetworkService::Init();
                    Sunset::NetworkService::Get().Host(7777, 2);

                    std::uniform_int_distribution<int> dist(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
                    worldParam.seed = dist(rng);
                    saves.emplace_back(worldParam);
                    Sunset::SaveSystem::Save(SAVE_PATH "GameSaved.bin", saves);

                    auto& app = Sunset::Application::GetApplication();
                    app.ClearLayer();
                    app.LoadLayer<GameLayer>(worldParam);
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
                Sunset::NetworkService::Init();
                Sunset::NetworkService::Get().Host(7777, 2);

                std::uniform_int_distribution<int> dist(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
                worldParam.seed = dist(rng);

                saves.emplace_back(worldParam);
                Sunset::SaveSystem::Save(SAVE_PATH "GameSaved.bin", saves);

                auto& app = Sunset::Application::GetApplication();
                app.ClearLayer();
                app.LoadLayer<GameLayer>(worldParam);
            })
            .Child(
                SRmGUI::SNew<SRmGUI::Text>()
                .Text("New World")
                .Color({0.1f, 0.1f, 0.1f, 1.f})
                .Padding({0.f, 0.f, 0.f, 40.f})
            ))
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

    // ImGui::Begin("Parameter");
    // ImGui::InputInt("Seed", &seed);
    // ImGui::SameLine();
    // if (ImGui::Button("Random"))
    // {
    //     std::uniform_int_distribution<int> dist(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    //     seed = dist(rng);
    // }
    // static char worldName[50] = {};
    // ImGui::InputText("World Name", worldName, 50);
    // if (ImGui::Button("Create World"))
    // {
    //     saves.emplace_back(worldName, seed);
    //     if (Sunset::SaveSystem::Save(SAVE_PATH "GameSaved.bin", saves))
    //     {
    //         LOG("SunsetCraft", info, "Save {} Success", std::string(worldName));
    //         Sunset::SaveSystem::CreateFolder(SAVE_PATH + saves.back().Name);
    //     }
    // }
    // for (const auto& save : saves)
    // {
    //     if (ImGui::Button(save.Name.c_str()))
    //     {
    //         Sunset::NetworkService::Init();
    //         Sunset::NetworkService::Get().Host(7777, 2);
    //
    //         auto& app = Sunset::Application::GetApplication();
    //         app.ClearLayer();
    //         app.LoadLayer<GameLayer>(WorldParam{save.Name, save.seed});
    //     }
    // }
    // ImGui::End();
}

bool MainMenu::OnEvent(const Sunset::Event::Type & event)
{
    Layer::OnEvent(event);

    // if (std::holds_alternative<Sunset::Event::Key>(event))
    // {
    //     const auto& key = std::get<Sunset::Event::KeyEvent>(event);
    //     if (key.action == Sunset::Event::Action::Press)
    //     {
    //         if (key.key == 259)
    //         {
    //             if (!worldParam.Name.empty())
    //                 worldParam.Name.pop_back();
    //
    //             return true;
    //         }
    //         worldParam.Name += key.key;
    //     }
    //     m_Text->SetText(worldParam.Name);
    //     return true;
    // }
    return false;
}
