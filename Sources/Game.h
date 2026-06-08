//
// Created by sunvy on 22/05/2026.
//

#ifndef SUNSETCRAFTV2_GAME_H
#define SUNSETCRAFTV2_GAME_H

#include "Core/Layer.h"

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


#endif //SUNSETCRAFTV2_GAME_H