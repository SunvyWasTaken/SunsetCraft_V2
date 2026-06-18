#version 330 core

in vec3 FragNormal;
flat in uint UVSide;

uniform sampler2D BlockTextures;

out vec4 FragColor;

vec2 ProcessUV(uint index)
{
    return vec2(index % 16u, index / 16u);
}

void main()
{
    vec4 color = vec4(1.0);
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diffuse = max(dot(normalize(FragNormal), lightDir), 0.55);

    vec4 texColor = texture(BlockTextures, ProcessUV(UVSide));

    FragColor = texColor * diffuse;
}