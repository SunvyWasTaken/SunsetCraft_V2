//
// Created by sunvy on 02/06/2026.
//

#include "NetworkLayer.h"

#include <imgui.h>

#include "Network/NetworkService.h"

namespace
{
    constexpr Sunset::ChannelId ChatChannel = 0;
    constexpr std::size_t MaxChatMessages = 64;

    struct ChatMessage
    {
        std::array<char, 120> Text{};
    };

    std::string ToString(const ChatMessage& message)
    {
        const auto end = std::find(message.Text.begin(), message.Text.end(), '\0');
        return { message.Text.begin(), end };
    }
}

NetworkLayer::NetworkLayer()
{
    Sunset::NetworkService::Get().RegisterHandler<ChatMessage>([this](Sunset::PeerId peer, const ChatMessage& message)
    {
        const std::string text = ToString(message);
        if (text.empty())
        {
            return;
        }

        m_ChatLog.push_back("Peer: " + text);
        if (m_ChatLog.size() > MaxChatMessages)
        {
            m_ChatLog.erase(m_ChatLog.begin());
        }
    });
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

    ImGui::SeparatorText("Chat");
    ImGui::BeginChild("##chat_log", ImVec2(0.f, 160.f), true);
    for (const std::string& message : m_ChatLog)
    {
        ImGui::TextWrapped("%s", message.c_str());
    }
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    {
        ImGui::SetScrollHereY(1.f);
    }
    ImGui::EndChild();

    ImGui::InputText("##message", m_Message.data(), m_Message.size());
    ImGui::SameLine();
    if (ImGui::Button("Send"))
    {
        ChatMessage message{};
        std::strncpy(message.Text.data(), m_Message.data(), message.Text.size() - 1);

        const std::string text = ToString(message);
        if (!text.empty())
        {
            const std::span messageSpan{ &message, 1 };
            Sunset::NetworkService::Get().Broadcast(ChatChannel, std::as_bytes(messageSpan));

            m_ChatLog.push_back("Me: " + text);
            if (m_ChatLog.size() > MaxChatMessages)
            {
                m_ChatLog.erase(m_ChatLog.begin());
            }
            m_Message.fill('\0');
        }
    }
    ImGui::End();
}
