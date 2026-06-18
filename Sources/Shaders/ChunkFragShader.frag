#version 330 core

in vec3 FragNormal;
flat in uint UVSide;
in vec2 FragUv;

uniform sampler2D BlockTextures;

out vec4 FragColor;

void main()
{
    vec4 color = vec4(1.0);
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diffuse = max(dot(normalize(FragNormal), lightDir), 0.55);

    vec2 tile = vec2(float(UVSide % 16u), float(UVSide / 16u));
    vec2 uv = (tile + FragUv) / 16.0;

    vec4 texColor = texture(BlockTextures, uv);

    FragColor = texColor * diffuse;
}