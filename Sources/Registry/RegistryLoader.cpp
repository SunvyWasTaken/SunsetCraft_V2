//
// Created by sunvy on 23/06/2026.
//

#include "RegistryLoader.h"

#include "ItemRegistry.h"
#include "BlockRegistry.h"
#include "TextureRegistry.h"

void RegistryLoader::Init()
{
    TextureBlockRegistry::Init();
    BlockRegistry::Init();
    ItemRegistry::Init();
}

void RegistryLoader::Destroy()
{
    BlockRegistry::Destroy();
    TextureBlockRegistry::Destroy();
    ItemRegistry::Destroy();
}
