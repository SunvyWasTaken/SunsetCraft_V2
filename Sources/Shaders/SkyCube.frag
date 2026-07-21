#version 330 core

in vec3 worldPos;

uniform float u_TimeOfDay;
uniform vec3 u_SunDirection;
uniform vec3 u_SunColor;

out vec4 FragColor;

const float uRadius = 500.0;

float DayAmount()
{
    float angle = (u_TimeOfDay - 6.0) / 24.0 * 6.2831853;
    return clamp(sin(angle) * 0.5 + 0.5, 0.0, 1.0);
}

void main()
{
    float h = worldPos.y / uRadius;
    float t = clamp((h + 1.0) * 0.5, 0.0, 1.0);

    float day = DayAmount();
    float horizon = 1.0 - t;
    float sunAmount = pow(max(dot(normalize(worldPos), normalize(u_SunDirection)), 0.0), 256.0);

    vec3 nightBottom = vec3(0.015, 0.025, 0.07);
    vec3 nightTop = vec3(0.002, 0.006, 0.025);
    vec3 dayBottom = vec3(0.55, 0.74, 0.98);
    vec3 dayTop = vec3(0.06, 0.30, 0.78);
    vec3 duskBottom = vec3(0.95, 0.36, 0.16);
    vec3 duskTop = vec3(0.12, 0.08, 0.25);

    float sunrise = 1.0 - min(abs(u_TimeOfDay - 6.0), abs(u_TimeOfDay - 18.0)) / 3.5;
    float dusk = clamp(sunrise, 0.0, 1.0) * day;

    vec3 nightColor = mix(nightBottom, nightTop, t);
    vec3 dayColor = mix(dayBottom, dayTop, t);
    vec3 duskColor = mix(duskBottom, duskTop, t);
    vec3 color = mix(nightColor, dayColor, day);
    color = mix(color, duskColor, dusk * horizon);
    color += u_SunColor * sunAmount * smoothstep(-0.02, 0.20, u_SunDirection.y);

    FragColor = vec4(color, 1.0);
}
