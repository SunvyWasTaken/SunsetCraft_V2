//
// Created by sunvy on 05/06/2026.
//

#include "MainMenu.h"

#include <imgui.h>

#include "Button.h"
#include "Game.h"
#include "HorizontalBox.h"
#include "Core/Application.h"
#include "Core/ApplicationSetting.h"
#include "Network/NetworkService.h"
#include "Render/Texture.h"
#include "Image.h"
#include "Overlay.h"
#include "Panel.h"
#include "Text.h"
#include "VerticalBox.h"

namespace
{
    std::unique_ptr<Sunset::Texture> m_Image = nullptr;
}

MainMenu::MainMenu()
    : Layer()
{
    m_Image = std::make_unique<Sunset::Texture>();
    m_Image->LoadImage(RESOURCES "Logo/Logo.png");

    const auto& setting = Sunset::Application::GetSetting();
    std::shared_ptr<SRmGUI::Image> overlay = std::make_shared<SRmGUI::Image>();
    overlay->SetPosition((setting.WindowSize/2) - (glm::ivec2{2172/3, 724/3} / 2) - glm::ivec2{0, 120});
    overlay->SetSize({2172/3, 724/3});
    overlay->SetImage(m_Image->GetId());


    auto box = std::make_shared<SRmGUI::VerticalBox>();
    box->SetPosition((setting.WindowSize/2) - glm::ivec2{75, -85});
    box->SetSize(glm::ivec2{150, 150});

    std::shared_ptr<SRmGUI::Button> Play = std::make_shared<SRmGUI::Button>();
    Play->SetCallback([&]()
    {
        Sunset::NetworkService::Init();
        Sunset::NetworkService::Get().Host(7777, 2);
        Sunset::Application::GetApplication().ClearLayer();
        //Sunset::Application::GetApplication().LoadOverlay<GameOverlay>();
        Sunset::Application::GetApplication().LoadLayer<GameLayer>();
    });
    auto PlayText = std::make_shared<SRmGUI::Text>();
    PlayText->SetColor({0.1f, 0.1, 0.1, 1.f});
    PlayText->SetText("Play");
    Play->AddChild(PlayText);
    // std::shared_ptr<SRmGUI::Button> Opt = std::make_shared<SRmGUI::Button>();
    std::shared_ptr<SRmGUI::Button> Quit = std::make_shared<SRmGUI::Button>();
    Quit->SetCallback([&]()
    {
        Sunset::Application::CloseApplication();
    });
    auto QuitText = std::make_shared<SRmGUI::Text>();
    QuitText->SetColor({0.1f, 0.1, 0.1, 1.f});
    QuitText->SetText("Quit");
    Quit->AddChild(QuitText);

    box->AddChild(Play);
    // box->AddChild(Opt);
    box->AddChild(Quit);

    std::shared_ptr<SRmGUI::Panel> panel = std::make_shared<SRmGUI::Panel>();
    panel->AddChild(overlay);
    panel->AddChild(box);

    AddToViewport(panel);
}

MainMenu::~MainMenu()
{
    m_Image.reset();
}

void MainMenu::OnUpdate(float dt)
{
    Layer::OnUpdate(dt);
}

void MainMenu::OnDraw()
{
    Layer::OnDraw();
}
