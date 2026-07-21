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
uniform vec3 u_SunColor;
uniform vec3 u_AmbientColor;

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

    float fresnel = pow(1.0 - max(dot(normal, viewDir), 0.0), 3.0);
    fresnel = clamp(fresnel, 0.0, 1.0);

    vec3 shallowColor = vec3(0.35, 0.70, 1.00);
    vec3 deepColor    = vec3(0.05, 0.28, 0.55);

    float colorMix = 0.5 + 0.5 * sin((FragWorldPos.x + FragWorldPos.z) * 0.08 + u_Time * 0.25);
    vec3 waterColor = deepColor;

    waterColor += fresnel * vec3(0.35, 0.55, 0.75);
    waterColor *= u_AmbientColor + u_SunColor * diffuse;
    waterColor += u_SunColor * specular;

    float alpha = mix(0.45, 0.68, fresnel);

    FragColor = vec4(waterColor, alpha);
}
