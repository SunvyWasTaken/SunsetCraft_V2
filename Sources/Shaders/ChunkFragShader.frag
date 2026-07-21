#version 330 core

in vec3 FragNormal;
in vec2 FragUv;
flat in uint UVSide;
flat in uint IsGrass;

uniform sampler2D BlockTextures;
uniform float u_TimeOfDay;
uniform vec3 u_SunDirection;
uniform vec3 u_SunColor;
uniform vec3 u_AmbientColor;

out vec4 FragColor;

const float AtlasTileCount = 16.0;

vec2 ProcessUV(uint index, vec2 localUV)
{
    vec2 atlasPos = vec2(float(index % 16u), float(index / 16u));
    return (atlasPos + localUV) / AtlasTileCount;
}

float DayAmount()
{
    float angle = (u_TimeOfDay - 6.0) / 24.0 * 6.2831853;
    return clamp(sin(angle) * 0.5 + 0.5, 0.0, 1.0);
}

void main()
{
    vec4 color = vec4(1.0);
    if (IsGrass != 0u)
    {
        color = vec4(0.47, 0.675, 0.188, 1.0);
    }

    vec4 texColor = texture(BlockTextures, ProcessUV(UVSide, FragUv));
    if (texColor.a < 0.1)
        discard;

    vec3 normal = normalize(FragNormal);
    vec3 lightDir = normalize(u_SunDirection);
    float sunVisibility = smoothstep(-0.08, 0.22, lightDir.y);
    float diffuse = max(dot(normal, lightDir), 0.0) * sunVisibility;

    float faceShade = mix(0.64, 1.0, max(normal.y, 0.0));
    vec3 nightTint = vec3(0.48, 0.55, 0.82);
    vec3 lightColor = u_AmbientColor + u_SunColor * diffuse;
    lightColor = mix(lightColor * nightTint, lightColor, DayAmount());

    FragColor = vec4(texColor.rgb * color.rgb * lightColor * faceShade, texColor.a * color.a);
}
