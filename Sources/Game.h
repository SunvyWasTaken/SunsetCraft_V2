//
// Created by sunvy on 22/05/2026.
//

#pragma once

#include "Inventaire.h"
#include "Sky.h"
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
    bool OnEvent(Sunset::Event::Type& event) override;
private:
    std::unique_ptr<Sunset::World> world = nullptr;
    Inventaire m_Inventaire;
    SkyCubed m_Sky;
};
