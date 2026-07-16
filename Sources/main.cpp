//
// Created by sunvy on 22/05/2026.
//

#include "Game.h"
#include "MainMenu.h"
#include "SunsetCraftModule.h"

#include "SunsetEngine.h"

int main()
{
#ifdef SUNSET_HEADLESS
    const Sunset::WindowSetting setting{"SunsetCraft V2", {1280, 720}, 30.f, true};
    Sunset::Application app{setting};
    Sunset::NetworkService::Init();
    Sunset::NetworkService::Get().Host(7777, 2);
    app.PushLayer<GameLayer>();
#else
    const Sunset::WindowSetting setting{"SunsetCraft V2", {1280, 720}};
    Sunset::Application app{setting};
    GameModule::InitGame(app);
#endif
    app.Run();
}

#include "Network/NetworkService.h"
