//
// Created by sunvy on 22/05/2026.
//

#include "Game.h"
#include "MainMenu.h"
#include "SunsetEngine.h"
#include "Network/NetworkService.h"

int main()
{
#ifdef SUNSET_HEADLESS
    const Sunset::ApplicationSetting setting{"SunsetCraft V2", {1280, 720}, 30.f, true};
    Sunset::Application app{setting};
    Sunset::NetworkService::Init();
    Sunset::NetworkService::Get().Host(7777, 2);
    app.PushLayer<GameLayer>();
#else
    const Sunset::ApplicationSetting setting{"SunsetCraft V2", {1280, 720}};
    Sunset::Application app{setting};

    INITLOG("SunsetCraft")

    app.PushLayer<MainMenu>();
#endif
    app.Run();
}
