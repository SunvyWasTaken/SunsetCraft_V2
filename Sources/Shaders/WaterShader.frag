#version 330 core

in vec3 FragNormal;
in vec2 FragUv;
in vec3 FragWorldPos;
flat in uint UVSide;

uniform sampler2D BlockTextures;
uniform float u_Time;

out vec4 FragColor;

const float AtlasTileCount = 16.0;

vec2 ProcessUV(uint index, vec2 localUV)
{
    vec2 atlasPos = vec2(float(index % 16u), float(index / 16u));
    return (atlasPos + localUV) / AtlasTileCount;
}

void main()
{
    vec2 ripple = vec2(
            sin(FragWorldPos.z * 0.65 + u_Time * 1.20),
            cos(FragWorldPos.x * 0.70 + u_Time * 1.10)
    ) * 0.015;

//    vec4 texColor = texture(BlockTextures, ProcessUV(UVSide, FragUv) + ripple / AtlasTileCount);
    vec4 waterColor = vec4(0.45, 0.72, 1.0, 0.55);
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diffuse = max(dot(normalize(FragNormal), lightDir), 0.55);
    float shimmer = 0.04 * sin((FragWorldPos.x + FragWorldPos.z) * 0.45 + u_Time * 1.6);

//    FragColor = vec4(waterColor.rgb * diffuse + texColor.rgb * 0.12 + shimmer, waterColor.a);
    FragColor = vec4(waterColor.rgb * diffuse + shimmer, waterColor.a);
}