#version 330 core

in vec3 FragNormal;
in vec2 FragUv;
flat in uint UVSide;

uniform sampler2D BlockTextures;

out vec4 FragColor;

const float AtlasTileCount = 16.0;

vec2 ProcessUV(uint index, vec2 localUV)
{
    vec2 atlasPos = vec2(float(index % 16u), float(index / 16u));
    return (atlasPos + localUV) / AtlasTileCount;
}

void main()
{
    vec4 color = vec4(1.0);
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diffuse = max(dot(normalize(FragNormal), lightDir), 0.55);

    vec4 texColor = texture(BlockTextures, ProcessUV(UVSide, FragUv));

    FragColor = texColor * color * diffuse;
}