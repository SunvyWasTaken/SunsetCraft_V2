//
// Created by sunvy on 05/06/2026.
//

#pragma once
#include "Core/Layer.h"

class MainMenu : public Sunset::Layer
{
public:
    void OnUpdate(float dt) override;
    void OnDraw() override;
};
