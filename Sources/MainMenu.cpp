//
// Created by sunvy on 05/06/2026.
//

#include "MainMenu.h"

#include <imgui.h>

#include "Game.h"
#include "Sandbox.h"
#include "Core/Application.h"
#include "Network/NetworkService.h"

void MainMenu::OnUpdate(float dt)
{
}

void MainMenu::OnDraw()
{
    ImGui::Begin("Menu");

    if (ImGui::Button("Start Server"))
    {
        Sunset::NetworkService::Get().Host(7777, 2);
        Sunset::Application::GetApplication().ClearLayer();
        Sunset::Application::GetApplication().LoadLayer<GameLayer>();
    }
    if (ImGui::Button("Join Server"))
    {
        Sunset::NetworkService::Get().Join({});
        Sunset::Application::GetApplication().ClearLayer();
        Sunset::Application::GetApplication().LoadLayer<GameLayer>();
    }
    if (ImGui::Button("Quit"))
    {
        Sunset::Application::CloseApplication();
    }
    ImGui::End();
}
