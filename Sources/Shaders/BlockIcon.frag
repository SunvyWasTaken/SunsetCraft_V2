#version 330 core

in vec3 v_Normal;
in vec2 v_UV;
flat in uint v_UVIndex;

uniform sampler2D u_BlockAtlas;

out vec4 FragColor;

const float AtlasTileCount = 16.0;

vec2 ProcessUV(uint index, vec2 localUV)
{
    vec2 atlasPos = vec2(float(index % 16u), float(index / 16u));
    return (atlasPos + localUV) / AtlasTileCount;
}

void main()
{
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diffuse = max(dot(normalize(v_Normal), lightDir), 0.55);

    vec4 texColor = texture(u_BlockAtlas, ProcessUV(v_UVIndex, v_UV));
    FragColor = vec4(texColor.rgb * diffuse, texColor.a);
}