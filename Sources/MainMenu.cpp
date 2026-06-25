//
// Created by sunvy on 05/06/2026.
//

#include "MainMenu.h"

#include <imgui.h>

#include "Game.h"
#include "Core/Application.h"
#include "Core/ApplicationSetting.h"
#include "Network/NetworkService.h"
#include "Widget/UIImage.h"

MainMenu::MainMenu()
{
    std::shared_ptr<Sunset::UIImage> image = std::make_shared<Sunset::UIImage>();
    image->LoadImage(RESOURCES "Logo/Logo.png");
    image->m_Bounds.position = Sunset::Application::GetSetting().WindowSize / 2;
    image->SetDesiredSize({2172, 724});
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
