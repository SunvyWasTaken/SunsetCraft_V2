#version 330 core

in vec3 FragNormal;
in vec2 FragUv;
flat in uint UVSide;
flat in uint IsGrass;

uniform sampler2D BlockTextures;
uniform float u_TimeOfDay;
uniform vec3 u_SunDirection;

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
    return smoothstep(0.04, 0.38, sin(angle));
}

float DawnDuskAmount()
{
    float dawn = abs(u_TimeOfDay - 6.0);
    float dusk = abs(u_TimeOfDay - 18.0);
    return 1.0 - smoothstep(0.0, 3.2, min(dawn, dusk));
}

vec3 SunColor()
{
    float day = DayAmount();
    float warm = DawnDuskAmount();

    vec3 nightLight = vec3(0.08, 0.11, 0.24);
    vec3 dayLight = vec3(1.00, 0.96, 0.84);
    vec3 duskLight = vec3(1.00, 0.43, 0.16);

    return mix(mix(nightLight, dayLight, day), duskLight, warm);
}

vec3 AmbientColor()
{
    float day = DayAmount();
    float warm = DawnDuskAmount();

    vec3 nightAmbient = vec3(0.025, 0.035, 0.090);
    vec3 dayAmbient = vec3(0.34, 0.39, 0.46);
    vec3 duskAmbient = vec3(0.24, 0.12, 0.08);

    return mix(mix(nightAmbient, dayAmbient, day), duskAmbient, warm * 0.65);
}

vec3 MoonColor()
{
    return vec3(0.18, 0.24, 0.42);
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

    vec3 moonDir = normalize(-u_SunDirection);
    float moonVisibility = smoothstep(-0.05, 0.35, moonDir.y);
    float moonDiffuse = max(dot(normal, moonDir), 0.0) * moonVisibility;

    float faceShade = mix(0.64, 1.0, max(normal.y, 0.0));
    vec3 lightColor = AmbientColor() + SunColor() * diffuse + MoonColor() * moonDiffuse;

    FragColor = vec4(texColor.rgb * color.rgb * lightColor * faceShade, texColor.a * color.a);
}
