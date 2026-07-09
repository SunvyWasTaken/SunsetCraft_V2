//
// Created by sunvy on 09/07/2026.
//

#include "SunsetCraftModule.h"

#include "Game.h"
#include "MainMenu.h"
#include "WorldParam.h"
#include "Log/Logger.h"

void GameModule::InitGame(Sunset::Application &app)
{
    INITLOG("SunsetCraft");
    app.PushLayer<GameLayer>(WorldParam{"World", 69});
}
