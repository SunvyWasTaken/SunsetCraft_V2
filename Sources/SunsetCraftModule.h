//
// Created by sunvy on 09/07/2026.
//

#pragma once

#include "Core/GameModule.h"

#ifdef _WIN32
    #define SUNSET_GAME_EXPORT extern "C" __declspec(dllexport)
#else
    #define SUNSET_GAME_EXPORT extern "C" __attribute__((visibility("default")))
#endif

struct SunsetCraftModule : public Sunset::IGameModule
{
    void Load(Sunset::Application& app) override;
    void Unload() override;
};

extern "C"
{
    SUNSET_GAME_EXPORT Sunset::IGameModule* SunsetCreateGameModule()
    {
        return new SunsetCraftModule;
    }

    SUNSET_GAME_EXPORT void SunsetDestroyGameModule(
        Sunset::IGameModule* module)
    {
        delete module;
    }
}
