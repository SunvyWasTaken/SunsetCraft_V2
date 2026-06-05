//
// Created by sunvy on 05/06/2026.
//

#include "MainMenu.h"

#include <imgui.h>

#include "Sandbox.h"
#include "Core/Application.h"

void MainMenu::OnUpdate(float dt)
{
}

void MainMenu::OnDraw()
{
    ImGui::Begin("Menu");
    if (ImGui::TreeNode("Main Menu"))
    {
        if (ImGui::Button("Start"))
        {
            Sunset::Application::GetApplication().ClearLayer();
            Sunset::Application::GetApplication().LoadLayer<Sandbox>();
        }
        if (ImGui::Button("Quit"))
        {
            Sunset::Application::CloseApplication();
        }
        ImGui::TreePop();
    }
    ImGui::End();
}
