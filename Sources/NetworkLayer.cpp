//
// Created by sunvy on 02/06/2026.
//

#include "NetworkLayer.h"

#include <imgui.h>

#include "Network/NetworkService.h"

NetworkLayer::NetworkLayer()
{

}

NetworkLayer::~NetworkLayer()
{
    Sunset::NetworkService::Get().Shutdown();
}

void NetworkLayer::OnUpdate(float dt)
{
    Sunset::NetworkService::Get().Update(dt);
}

void NetworkLayer::OnDraw()
{
    ImGui::Begin("Network");
    if (ImGui::Button("Start Server"))
    {
        Sunset::NetworkService::Get().Host(7777, 2);
    }
    if (ImGui::Button("Join Server"))
    {
        Sunset::NetworkService::Get().Join({});
    }
    ImGui::End();
}
