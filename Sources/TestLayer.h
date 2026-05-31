//
// Created by sunvy on 24/05/2026.
//

#ifndef SUNSETCRAFTV2_TESTLAYER_H
#define SUNSETCRAFTV2_TESTLAYER_H

#include "Core/Layer.h"

class TestLayer : public Sunset::Layer
{
    public:
    TestLayer();
    ~TestLayer() override;
    void OnUpdate(float dt) override;
    void OnDraw() override;
};

#endif //SUNSETCRAFTV2_TESTLAYER_H