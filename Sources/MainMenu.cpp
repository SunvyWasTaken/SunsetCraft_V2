//
// Created by sunvy on 05/06/2026.
//

#include "MainMenu.h"

#include "Button.h"
#include "Game.h"
#include "Core/Application.h"
#include "Core/ApplicationSetting.h"
#include "Network/NetworkService.h"
#include "Render/Texture.h"
#include "Image.h"
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

    const glm::ivec2 LogoSize = {2172/3, 724/3};

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
                .OnClicked([&]()
                {
                    Sunset::NetworkService::Init();
                    Sunset::NetworkService::Get().Host(7777, 2);

                    auto& app = Sunset::Application::GetApplication();
                    app.ClearLayer();
                    app.LoadLayer<GameLayer>();
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
