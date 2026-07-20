//
// Created by sunvy on 09/07/2026.
//

#include "SunsetCraftModule.h"

#include "Game.h"
#include "MainMenu.h"
#include "WorldParam.h"
#include "Core/Application.h"
#include "Log/Logger.h"

void SunsetCraftModule::Load(Sunset::Application &app)
{
    app.PushLayer<GameLayer>(WorldParam{"World", 69});
}

void SunsetCraftModule::Unload()
{
}


