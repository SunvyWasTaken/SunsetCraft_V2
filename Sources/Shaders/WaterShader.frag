#version 330 core

in vec3 FragNormal;
in vec2 FragUv;
in vec3 FragWorldPos;
flat in uint UVSide;
flat in uint FaceSide;

uniform sampler2D BlockTextures;
uniform float u_Time;
uniform vec3 u_CameraPos;
uniform float u_TimeOfDay;
uniform vec3 u_SunDirection;

out vec4 FragColor;

const float AtlasTileCount = 16.0;

vec2 ProcessUV(uint index, vec2 localUV)
{
    vec2 atlasPos = vec2(float(index % 16u), float(index / 16u));
    return (atlasPos + localUV) / AtlasTileCount;
}

float Wave(vec2 p, vec2 dir, float freq, float speed, float amp)
{
    return sin(dot(p, normalize(dir)) * freq + u_Time * speed) * amp;
}

float WaterHeight(vec2 p)
{
    float h = 0.0;

    h += Wave(p, vec2( 1.0,  0.3), 1.20, 1.40, 0.025);
    h += Wave(p, vec2(-0.4,  1.0), 0.85, 1.05, 0.018);
    h += Wave(p, vec2( 0.7, -0.6), 1.75, 1.90, 0.010);
    h += Wave(p, vec2(-1.0, -0.2), 2.40, 2.30, 0.006);

    return h;
}

vec3 ComputeWaterNormal(vec2 p)
{
    float e = 0.08;

    float hL = WaterHeight(p - vec2(e, 0.0));
    float hR = WaterHeight(p + vec2(e, 0.0));
    float hD = WaterHeight(p - vec2(0.0, e));
    float hU = WaterHeight(p + vec2(0.0, e));

    return normalize(vec3(hL - hR, 2.0 * e, hD - hU));
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

    vec3 nightAmbient = vec3(0.018, 0.030, 0.085);
    vec3 dayAmbient = vec3(0.30, 0.38, 0.48);
    vec3 duskAmbient = vec3(0.22, 0.10, 0.08);

    return mix(mix(nightAmbient, dayAmbient, day), duskAmbient, warm * 0.7);
}

vec3 MoonColor()
{
    return vec3(0.16, 0.22, 0.42);
}

void main()
{
    vec3 viewDir = normalize(u_CameraPos - FragWorldPos);
    vec3 lightDir = normalize(u_SunDirection);
    float sunVisibility = smoothstep(-0.08, 0.22, lightDir.y);

    vec3 normal = normalize(FragNormal);

    // Seulement la face du dessus utilise la normale dynamique.
    // Chez toi side == 3 correspond à Y+.
    if (FaceSide == 3u)
    {
        normal = ComputeWaterNormal(FragWorldPos.xz);
    }

    float diffuse = max(dot(normal, lightDir), 0.0) * sunVisibility;

    vec3 halfDir = normalize(lightDir + viewDir);
    float specular = pow(max(dot(normal, halfDir), 0.0), 96.0) * 0.45 * sunVisibility;

    vec3 moonDir = normalize(-u_SunDirection);
    float moonVisibility = smoothstep(-0.05, 0.35, moonDir.y);
    float moonDiffuse = max(dot(normal, moonDir), 0.0) * moonVisibility;
    vec3 moonHalfDir = normalize(moonDir + viewDir);
    float moonSpecular = pow(max(dot(normal, moonHalfDir), 0.0), 128.0) * 0.18 * moonVisibility;

    float fresnel = pow(1.0 - max(dot(normal, viewDir), 0.0), 3.0);
    fresnel = clamp(fresnel, 0.0, 1.0);

    vec3 shallowColor = vec3(0.35, 0.70, 1.00);
    vec3 deepColor    = vec3(0.05, 0.28, 0.55);

    vec3 waterColor = deepColor;

    waterColor += fresnel * vec3(0.35, 0.55, 0.75);
    waterColor *= AmbientColor() + SunColor() * diffuse + MoonColor() * moonDiffuse;
    waterColor += SunColor() * specular + MoonColor() * moonSpecular;

    float alpha = mix(0.45, 0.68, fresnel);

    FragColor = vec4(waterColor, alpha);
}
