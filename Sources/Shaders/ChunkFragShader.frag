#version 330 core

in vec3 FragNormal;
in vec2 FragUv;
in vec3 FragWorldPos;
in float FragAO;
flat in uint UVSide;
flat in uint IsGrass;

uniform sampler2D BlockTextures;
uniform sampler2D u_ShadowMap;
uniform vec3 u_CameraPos;
uniform float u_TimeOfDay;
uniform float u_CloudTime;
uniform vec3 u_SunDirection;
uniform mat4 u_LightSpaceMatrix;
uniform int u_ShadowsEnabled;

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

vec3 FogColor()
{
    float day = DayAmount();
    float dusk = DawnDuskAmount() * max(day, 0.35);

    vec3 nightFog = vec3(0.018, 0.030, 0.095);
    vec3 dayFog = vec3(0.58, 0.76, 1.00);
    vec3 duskFog = vec3(0.95, 0.34, 0.16);

    return mix(mix(nightFog, dayFog, day), duskFog, dusk * 0.85);
}

float FogAmount(vec3 worldPos)
{
    float distanceFog = smoothstep(80.0, 260.0, distance(u_CameraPos, worldPos));
    float groundMist = exp(-max(worldPos.y + 2.0, 0.0) * 0.070) * 0.28;
    float waterMist = exp(-abs(worldPos.y) * 0.22) * 0.16;

    return clamp(distanceFog + max(groundMist, waterMist) * (1.0 - distanceFog * 0.45), 0.0, 0.88);
}

float ShadowAmount(vec3 worldPos, vec3 normal, vec3 lightDir)
{
    if (u_ShadowsEnabled == 0)
        return 0.0;

    vec4 lightSpacePosition = u_LightSpaceMatrix * vec4(worldPos, 1.0);
    vec3 projectionCoords = lightSpacePosition.xyz / lightSpacePosition.w;
    projectionCoords = projectionCoords * 0.5 + 0.5;

    if (projectionCoords.z > 1.0 || projectionCoords.x < 0.0 || projectionCoords.x > 1.0 || projectionCoords.y < 0.0 || projectionCoords.y > 1.0)
        return 0.0;

    float bias = max(0.004 * (1.0 - dot(normal, lightDir)), 0.0015);
    vec2 texelSize = 1.0 / vec2(textureSize(u_ShadowMap, 0));
    float shadow = 0.0;

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float closestDepth = texture(u_ShadowMap, projectionCoords.xy + vec2(x, y) * texelSize).r;
            shadow += projectionCoords.z - bias > closestDepth ? 1.0 : 0.0;
        }
    }

    return shadow / 9.0;
}

float Hash(vec2 p)
{
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

float Noise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(
        mix(Hash(i + vec2(0.0, 0.0)), Hash(i + vec2(1.0, 0.0)), u.x),
        mix(Hash(i + vec2(0.0, 1.0)), Hash(i + vec2(1.0, 1.0)), u.x),
        u.y);
}

float Fbm(vec2 p)
{
    float value = 0.0;
    float amplitude = 0.5;

    for (int i = 0; i < 4; ++i)
    {
        value += Noise(p) * amplitude;
        p = p * 2.03 + vec2(17.7, 9.2);
        amplitude *= 0.5;
    }

    return value;
}

float CloudShadow(vec3 worldPos, vec3 lightDir)
{
    if (lightDir.y <= 0.05)
        return 0.0;

    float cloudHeight = 128.0;
    float travel = max((cloudHeight - worldPos.y) / lightDir.y, 0.0);
    vec2 cloudPos = (worldPos + lightDir * travel).xz;
    vec2 wind = vec2(u_CloudTime * 0.010, u_CloudTime * 0.006);

    float lower = smoothstep(0.56, 0.92, Fbm(cloudPos * 0.0072 + wind));
    float middle = smoothstep(0.62, 0.94, Fbm(cloudPos * 0.0105 + wind * 1.7 + vec2(4.0, -2.0)));
    float density = clamp(lower * 0.65 + middle * 0.35, 0.0, 1.0);

    return density * DayAmount() * smoothstep(0.05, 0.30, lightDir.y);
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
    float shadow = ShadowAmount(FragWorldPos, normal, lightDir) * sunVisibility;
    float cloudShadow = CloudShadow(FragWorldPos, lightDir);

    vec3 moonDir = normalize(-u_SunDirection);
    float moonVisibility = smoothstep(-0.05, 0.35, moonDir.y);
    float moonDiffuse = max(dot(normal, moonDir), 0.0) * moonVisibility;

    float faceShade = mix(0.64, 1.0, max(normal.y, 0.0));
    vec3 lightColor = AmbientColor() + SunColor() * diffuse * mix(1.0, 0.28, shadow) * mix(1.0, 0.76, cloudShadow) + MoonColor() * moonDiffuse;
    vec3 finalColor = texColor.rgb * color.rgb * lightColor * faceShade * FragAO;
    finalColor = mix(finalColor, FogColor(), FogAmount(FragWorldPos));

    FragColor = vec4(finalColor, texColor.a * color.a);
}
