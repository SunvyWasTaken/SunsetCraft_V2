//
// Created by sunvy on 22/05/2026.
//

#pragma once

#include "ShadowMap.h"
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
    void Init() override;
    void OnUpdate(float dt) override;
    void OnDraw() override;
    bool OnEvent(const Sunset::Event::Type& event) override;
private:
    SkyCubed m_Sky;
    ShadowMap m_ShadowMap;
};
