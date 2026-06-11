//
// Created by sunvy on 22/05/2026.
//

#pragma once

#include "Core/Layer.h"

class GameOverlay : public Sunset::Layer
{
public:
    GameOverlay();
    ~GameOverlay() override;

    void OnUpdate(float dt) override;
    void OnDraw() override;
};

class GameLayer : public Sunset::Layer
{
public:
    GameLayer();
    ~GameLayer() override;
    void OnUpdate(float dt) override;
    void OnDraw() override;
private:
    std::unique_ptr<Sunset::World> world = nullptr;
};
