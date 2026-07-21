#version 330 core

in vec3 worldPos;

uniform float u_TimeOfDay;
uniform float u_CloudTime;
uniform vec3 u_SunDirection;

out vec4 FragColor;

const float uRadius = 500.0;

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
    return mix(vec3(1.00, 0.96, 0.78), vec3(1.00, 0.45, 0.16), DawnDuskAmount());
}

vec3 MoonColor()
{
    return vec3(0.72, 0.82, 1.00);
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

    for (int i = 0; i < 5; ++i)
    {
        value += Noise(p) * amplitude;
        p = p * 2.03 + vec2(17.7, 9.2);
        amplitude *= 0.5;
    }

    return value;
}

float CloudLayer(vec2 p, float scale, float threshold)
{
    float base = Fbm(p * scale);
    float detail = Fbm(p * scale * 3.1 + vec2(11.0, -7.0)) * 0.35;
    return smoothstep(threshold, 1.0, base + detail);
}

vec4 VolumetricClouds(vec3 skyDir, vec3 baseSkyColor)
{
    if (skyDir.y <= 0.02)
        return vec4(baseSkyColor, 0.0);

    vec2 wind = vec2(u_CloudTime * 0.010, u_CloudTime * 0.006);
    vec2 p = skyDir.xz / max(skyDir.y, 0.08);

    float lower = CloudLayer(p + wind, 0.72, 0.58);
    float middle = CloudLayer(p * 1.35 + wind * 1.7 + vec2(4.0, -2.0), 0.58, 0.62);
    float upper = CloudLayer(p * 2.10 - wind * 0.75 + vec2(-9.0, 6.0), 0.42, 0.68);

    float density = clamp(lower * 0.62 + middle * 0.38 + upper * 0.18, 0.0, 1.0);
    density *= smoothstep(0.03, 0.22, skyDir.y) * (1.0 - smoothstep(0.78, 1.0, skyDir.y) * 0.20);

    vec3 sunDir = normalize(u_SunDirection);
    float day = DayAmount();
    float sunBackLight = pow(max(dot(skyDir, sunDir), 0.0), 6.0);
    float directLight = clamp(dot(vec3(0.25, 0.92, 0.25), sunDir) * 0.5 + 0.5, 0.0, 1.0);

    vec3 shadowCloud = mix(vec3(0.20, 0.23, 0.31), vec3(0.58, 0.62, 0.68), day);
    vec3 litCloud = mix(vec3(0.55, 0.60, 0.76), vec3(1.0, 0.96, 0.88), day);
    litCloud = mix(litCloud, SunColor(), DawnDuskAmount() * 0.30);

    vec3 cloudColor = mix(shadowCloud, litCloud, directLight);
    cloudColor += SunColor() * sunBackLight * 0.35 * day;
    cloudColor = mix(baseSkyColor, cloudColor, density);

    return vec4(cloudColor, density);
}

void main()
{
    float h = worldPos.y / uRadius;
    float t = clamp((h + 1.0) * 0.5, 0.0, 1.0);

    float day = DayAmount();
    float horizon = 1.0 - t;

    vec3 nightBottom = vec3(0.018, 0.030, 0.095);
    vec3 nightTop = vec3(0.002, 0.006, 0.030);
    vec3 dayBottom = vec3(0.58, 0.76, 1.00);
    vec3 dayTop = vec3(0.055, 0.28, 0.78);
    vec3 duskBottom = vec3(1.00, 0.40, 0.14);
    vec3 duskTop = vec3(0.16, 0.065, 0.20);

    float dusk = DawnDuskAmount() * max(day, 0.35);

    vec3 nightColor = mix(nightBottom, nightTop, t);
    vec3 dayColor = mix(dayBottom, dayTop, t);
    vec3 duskColor = mix(duskBottom, duskTop, t);
    vec3 color = mix(nightColor, dayColor, day);
    color = mix(color, duskColor, dusk * horizon);

    vec3 skyDir = normalize(worldPos);
    vec4 clouds = VolumetricClouds(skyDir, color);
    color = clouds.rgb;

    vec3 sunDir = normalize(u_SunDirection);
    float sunDot = dot(skyDir, sunDir);
    float sunVisible = smoothstep(-0.02, 0.08, sunDir.y);
    float sunDisk = smoothstep(0.9992, 0.9998, sunDot);
    float sunGlow = pow(max(sunDot, 0.0), 192.0);

    color += SunColor() * sunGlow * 1.4 * sunVisible;
    color = mix(color, vec3(1.0, 0.93, 0.68), sunDisk * sunVisible);

    vec3 moonDir = normalize(-u_SunDirection);
    float moonDot = dot(skyDir, moonDir);
    float moonVisible = smoothstep(-0.05, 0.18, moonDir.y);
    float moonDisk = smoothstep(0.9988, 0.9995, moonDot);
    float moonGlow = pow(max(moonDot, 0.0), 96.0);

    color += MoonColor() * moonGlow * 0.18 * moonVisible;
    color = mix(color, MoonColor(), moonDisk * moonVisible);

    FragColor = vec4(color, 1.0);
}
