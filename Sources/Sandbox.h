//
// Created by sunvy on 04/06/2026.
//

#pragma once

#include "Core/Layer.h"

namespace Sunset
{
    class World;
}

class Sandbox : public Sunset::Layer
{
public:
    Sandbox();

    virtual ~Sandbox();

    void OnUpdate(float deltatime);

    void OnDraw() override;

private:
    std::unique_ptr<Sunset::World> world;
};
