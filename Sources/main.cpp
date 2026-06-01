//
// Created by sunvy on 22/05/2026.
//

#include "Game.h"
#include "SunsetEngine.h"

int main()
{
    const Sunset::ApplicationSetting setting{{1280, 720}, "SunsetCraft V2"};
    Sunset::Application app{setting};

    app.PushLayer<GameLayer>();

    app.Run();
}
