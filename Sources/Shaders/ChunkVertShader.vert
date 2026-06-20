#version 330 core

layout(location = 0) in uint data;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;

out vec3 FragNormal;
out vec2 FragUv;
flat out uint UVSide;
flat out uint IsGrass;

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
    return (v >> 20) & 0xFFu;
}

const vec3 faceVerts[36] = vec3[](
        vec3(0,0,0), vec3(0,1,0), vec3(0,1,1), vec3(0,1,1), vec3(0,0,1), vec3(0,0,0), // -X
        vec3(1,0,1), vec3(1,1,1), vec3(1,1,0), vec3(1,1,0), vec3(1,0,0), vec3(1,0,1), //  X
        vec3(0,0,0), vec3(0,0,1), vec3(1,0,1), vec3(1,0,1), vec3(1,0,0), vec3(0,0,0), // -Y
        vec3(0,1,0), vec3(1,1,0), vec3(1,1,1), vec3(1,1,1), vec3(0,1,1), vec3(0,1,0), //  Y
        vec3(1,0,0), vec3(1,1,0), vec3(0,1,0), vec3(0,1,0), vec3(0,0,0), vec3(1,0,0), // -Z
        vec3(0,0,1), vec3(0,1,1), vec3(1,1,1), vec3(1,1,1), vec3(1,0,1), vec3(0,0,1)  //  Z
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

void main()
{
    vec3 blockPos = DecodePos(data);
    uint side = DecodeSide(data);
    UVSide = DecodeUV(data);

    int vertIndex = gl_VertexID % 6;
    int offset = faceOffset(side);

    vec3 vertPos = faceVerts[offset + vertIndex];

    vec3 position = blockPos + vertPos - vec3(0.0, 255.0, 0.0);

    gl_Position = projection * view * model * vec4(position, 1.0);
    FragNormal = faceNormals[int(side)];
    FragUv = faceUVs[vertIndex];
    IsGrass = (data >> 28) & 0x1u;
}