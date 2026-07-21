#version 330 core

in vec3 worldPos;

uniform float u_TimeOfDay;
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
