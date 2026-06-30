//
// Created by sunvy on 23/06/2026.
//

#include "RegistryLoader.h"

#include "ItemRegistry.h"
#include "BlockRegistry.h"
#include "TextureRegistry.h"
#include "Inventory/BlockIconRender.h"

void RegistryLoader::Init()
{
    TextureBlockRegistry::Init();
    BlockRegistry::Init();
    ItemRegistry::Init();
    BlockIconRender::Init();
}

void RegistryLoader::Destroy()
{
    BlockIconRender::Destroy();
    BlockRegistry::Destroy();
    TextureBlockRegistry::Destroy();
    ItemRegistry::Destroy();
}
