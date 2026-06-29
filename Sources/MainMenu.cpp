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


    std::shared_ptr<SRmGUI::HorizontalBox> box = std::make_shared<SRmGUI::HorizontalBox>();
    box->SetPosition((setting.WindowSize/2) - glm::ivec2{75, -50});
    box->SetSize(glm::ivec2{150, 50});

    std::shared_ptr<SRmGUI::Button> Play = std::make_shared<SRmGUI::Button>();
    box->AddChild(Play);

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
    ImGui::Begin("Menu");

    if (ImGui::Button("Start Server"))
    {
        Sunset::NetworkService::Init();
        Sunset::NetworkService::Get().Host(7777, 2);
        Sunset::Application::GetApplication().ClearLayer();
        Sunset::Application::GetApplication().LoadOverlay<GameOverlay>();
        Sunset::Application::GetApplication().LoadLayer<GameLayer>();
    }
    if (ImGui::Button("Join Server"))
    {
        Sunset::NetworkService::Init();
        Sunset::NetworkService::Get().Join({});
        Sunset::Application::GetApplication().ClearLayer();
        Sunset::Application::GetApplication().LoadOverlay<GameOverlay>();
        Sunset::Application::GetApplication().LoadLayer<GameLayer>();
    }
    if (ImGui::Button("Quit"))
    {
        Sunset::Application::CloseApplication();
    }
    ImGui::End();
}
