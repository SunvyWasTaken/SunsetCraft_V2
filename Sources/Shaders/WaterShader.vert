#version 330 core

layout(location = 0) in uint data;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;
uniform float u_Time;

out vec3 FragNormal;
out vec2 FragUv;
out vec3 FragWorldPos;
flat out uint UVSide;
flat out uint FaceSide;

vec3 DecodePos(uint v)
{
    return vec3(
        float((v >> 0) & 0xFu),
        float((v >> 4) & 0x1FFu),
        float((v >> 13) & 0xFu)
    );
}

uint DecodeSide(uint v)
{
    return (v >> 17) & uint(0x7);
}

uint DecodeUV(uint v)
{
    return (v >> 20) & uint(0xFFu);
}

const float y = 0.85;

const vec3 faceVerts[36] = vec3[](
        vec3(0,0,0), vec3(0,y,0), vec3(0,y,1), vec3(0,y,1), vec3(0,0,1), vec3(0,0,0), // -X
        vec3(1,0,0), vec3(1,0,1), vec3(1,y,1), vec3(1,y,1), vec3(1,y,0), vec3(1,0,0), //  X
        vec3(0,0,0), vec3(0,0,1), vec3(1,0,1), vec3(1,0,1), vec3(1,0,0), vec3(0,0,0), // -Y
        vec3(0,y,0), vec3(1,y,0), vec3(1,y,1), vec3(1,y,1), vec3(0,y,1), vec3(0,y,0), //  Y
        vec3(0,0,0), vec3(1,0,0), vec3(1,y,0), vec3(1,y,0), vec3(0,y,0), vec3(0,0,0), // -Z
        vec3(0,0,1), vec3(0,y,1), vec3(1,y,1), vec3(1,y,1), vec3(1,0,1), vec3(0,0,1)  //  Z
);

const vec3 faceNormals[6] = vec3[](
        vec3(-1, 0, 0), vec3(1, 0, 0), vec3(0, -1, 0), vec3(0, 1, 0), vec3(0, 0, -1), vec3(0, 0, 1)
);

const vec2 faceUVs[6] = vec2[](
        vec2(0,0), vec2(0,1), vec2(1,1), vec2(1,1), vec2(1,0), vec2(0,0)
);

int faceOffset(uint side)
{
    if (side == 0u) return 0;
    if (side == 1u) return 6;
    if (side == 2u) return 12;
    if (side == 3u) return 18;
    if (side == 4u) return 24;
    return 30;
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

vec2 WaterFlow(vec2 p)
{
    vec2 flow = vec2(0.0);

    flow.x += sin(p.y * 0.75 + u_Time * 0.9) * 0.008;
    flow.y += cos(p.x * 0.70 + u_Time * 0.8) * 0.008;

    return flow;
}

void main()
{
    vec3 blockPos = DecodePos(data);
    uint side = DecodeSide(data);
    UVSide = DecodeUV(data);

    int vertIndex = gl_VertexID % 6;
    int offset = faceOffset(side);

    vec3 vertPos = faceVerts[offset + vertIndex];

    vec3 position = blockPos + vertPos - vec3(0.0, 255.0, 0.0);
    vec3 worldPosition = vec3(model * vec4(position, 1.0));

    float topInfluence = smoothstep(0.05, y, vertPos.y);

    float h = WaterHeight(worldPosition.xz);
    vec2 flow = WaterFlow(worldPosition.xz);

    position.y += h * topInfluence;

    // Très léger déplacement horizontal, sinon ça devient caoutchouc.
    position.xz += flow * topInfluence;

    worldPosition = vec3(model * vec4(position, 1.0));

    gl_Position = projection * view * vec4(worldPosition, 1.0);
    FragNormal = faceNormals[int(side)];
    FragUv = faceUVs[vertIndex];
    FragWorldPos = worldPosition;
    FaceSide = side;
}