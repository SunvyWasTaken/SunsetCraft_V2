//
// Created by sunvy on 05/06/2026.
//

#pragma once
#include "Core/Layer.h"

class MainMenu : public Sunset::Layer
{
public:
    ~MainMenu() override;
    void Init() override;
    void OnUpdate(float dt) override;
    void OnDraw() override;
    bool OnEvent(Sunset::Event::Type &event) override;
};
