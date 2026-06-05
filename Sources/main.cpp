//
// Created by sunvy on 22/05/2026.
//

#include "Game.h"
#include "MainMenu.h"
#include "NetworkLayer.h"
#include "Sandbox.h"
#include "SunsetEngine.h"

int main()
{
    const Sunset::ApplicationSetting setting{{1280, 720}, "SunsetCraft V2"};
    Sunset::Application app{setting};

    // app.PushLayer<NetworkLayer>();
    // app.PushLayer<GameLayer>();

    app.PushLayer<MainMenu>();

    app.Run();
}
