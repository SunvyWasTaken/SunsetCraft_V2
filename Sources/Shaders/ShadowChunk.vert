#version 330 core

layout(location = 0) in uint data;

uniform mat4 model;
uniform mat4 u_LightSpaceMatrix;

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

const vec3 faceVerts[36] = vec3[](
        vec3(0,0,0), vec3(0,1,0), vec3(0,1,1), vec3(0,1,1), vec3(0,0,1), vec3(0,0,0),
        vec3(1,0,1), vec3(1,1,1), vec3(1,1,0), vec3(1,1,0), vec3(1,0,0), vec3(1,0,1),
        vec3(0,0,0), vec3(0,0,1), vec3(1,0,1), vec3(1,0,1), vec3(1,0,0), vec3(0,0,0),
        vec3(0,1,0), vec3(1,1,0), vec3(1,1,1), vec3(1,1,1), vec3(0,1,1), vec3(0,1,0),
        vec3(1,0,0), vec3(1,1,0), vec3(0,1,0), vec3(0,1,0), vec3(0,0,0), vec3(1,0,0),
        vec3(0,0,1), vec3(0,1,1), vec3(1,1,1), vec3(1,1,1), vec3(1,0,1), vec3(0,0,1)
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
    int vertIndex = gl_VertexID % 6;
    int offset = faceOffset(side);

    vec3 position = blockPos + faceVerts[offset + vertIndex] - vec3(0.0, 255.0, 0.0);
    gl_Position = u_LightSpaceMatrix * model * vec4(position, 1.0);
}
