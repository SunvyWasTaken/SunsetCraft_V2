//
// Created by sunvy on 22/05/2026.
//

#pragma once

#include "Inventory/Inventory.h"
#include "Sky.h"
#include "Core/Layer.h"

namespace Sunset
{
    class World;
}

struct WorldParam;

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
    explicit GameLayer(WorldParam param);
    ~GameLayer() override;
    void OnUpdate(float dt) override;
    void OnDraw() override;
    bool OnEvent(Sunset::Event::Type& event) override;
private:
    std::unique_ptr<Sunset::World> world;
    Inventory m_Inventory;
    SkyCubed m_Sky;
};
