//
// Created by sunvy on 02/06/2026.
//

#pragma once

#include "Core/Layer.h"

class NetworkLayer : public Sunset::Layer
{
public:
    NetworkLayer();

    ~NetworkLayer() override;

    void OnUpdate(float dt) override;

    void OnDraw() override;

private:
    std::array<char, 120> m_Message{};
    std::vector<std::string> m_ChatLog;
};
