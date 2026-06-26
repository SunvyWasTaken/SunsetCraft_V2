//
// Created by sunvy on 05/06/2026.
//

#include "MainMenu.h"

#include <imgui.h>

#include "Game.h"
#include "Core/Application.h"
#include "Core/ApplicationSetting.h"
#include "Network/NetworkService.h"
#include "Render/Texture.h"
#include "Widget/UIImage.h"

class LogoImage final : public Sunset::UIImage
{
public:
    explicit LogoImage()
        : UIImage()
    {
        SetDesiredSize({220, 56});
        m_Texture = std::make_shared<Sunset::Texture>();
        m_Texture->LoadImage(RESOURCES "Logo/Logo.png");
    }

    void Arrange(const Sunset::Rectangle& parentRect) override
    {
        const glm::ivec2 size = GetDesiredSize();

        m_Bounds.position = {
            parentRect.position.x + 40,
            parentRect.position.y + 40
        };

        m_Bounds.size = size;
    }
};

MainMenu::MainMenu()
{
    std::shared_ptr<LogoImage> image = std::make_shared<LogoImage>();
    // image->LoadImage(RESOURCES "Logo/Logo.png");
    image->m_Bounds.position = Sunset::Application::GetSetting().WindowSize / 2;
    image->SetDesiredSize({250, 50});
    AddToViewport(image);
}

void MainMenu::OnUpdate(float dt)
{
}

void MainMenu::OnDraw()
{
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
