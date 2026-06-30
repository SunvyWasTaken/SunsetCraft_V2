//
// Created by sunvy on 30/06/2026.
//

#include "BlockIconRender.h"

#include "Render/FrameBuffer.h"
#include "Render/RenderCommande.h"
#include "Render/Shader.h"
#include "Render/Texture.h"

namespace
{
    std::shared_ptr<Sunset::Shader> blockIconShader = nullptr;
    Sunset::Texture TextureID;
}

void BlockIconRender::Init()
{
    blockIconShader = std::make_shared<Sunset::Shader>(SHADERS_PATH "BlockIcon.vert", SHADERS_PATH "BlockIcon.frag");
    Sunset::FrameBuffer iconFbo({ .width = 64, .height = 64, .useDepth = true });

    // À chaque fois que tu veux générer/mettre à jour l’icône :
    Sunset::RenderCommande::BeginTarget(iconFbo, glm::vec4(0.0f));

    // Important : utiliser une caméra spéciale icône ici.
    Sunset::RenderCommande::UseCamera(iconCamera);

    // Ton cube/bloc/objet custom utilise son Material + shader custom.
    Sunset::RenderCommande::Submit(blockDrawable, blockTransform);

    Sunset::RenderCommande::EndTarget();

    // Texture OpenGL finale à afficher dans ton UI / atlas / système d’icônes :
    std::uint32_t iconTexture = iconFbo.GetColorAttachment();
}

void BlockIconRender::Destroy()
{
    blockIconShader.reset();
}

std::uint32_t BlockIconRender::GetTexture()
{
    return TextureID.GetId();
}

glm::vec4 BlockIconRender::GetIconUv(Item::Id id)
{
    return {};
}
