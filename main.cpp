//
// Created by sunvy on 22/05/2026.
//

#include "SunsetEngine.h"

int main()
{
    const SunsetEngine::ApplicationSetting setting{{1280, 720}, "SunsetCraft V2"};
    SunsetEngine::Application app{setting};
    app.Run();
}
