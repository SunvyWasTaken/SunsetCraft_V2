#version 330 core

layout(location = 0) in uint vData;

uniform vec3 location;
uniform mat4 projection;
uniform mat4 view;

out vec3 FragNormal;
out vec2 FragUV;
flat out uint BlockType;

uint DecodeX(uint data) { return data & 15u; }
uint DecodeZ(uint data) { return (data >> 4u) & 15u; }
uint DecodeY(uint data) { return (data >> 8u) & 255u; }
uint DecodeFace(uint data) { return (data >> 16u) & 7u; }
uint DecodeCorner(uint data) { return (data >> 19u) & 7u; }
uint DecodeBlock(uint data) { return (data >> 22u) & 255u; }

const vec3 faceVerts[36] = vec3[](
    vec3(0,0,0), vec3(1,1,0), vec3(1,0,0), vec3(0,0,0), vec3(0,1,0), vec3(1,1,0),
    vec3(1,0,1), vec3(0,1,1), vec3(0,0,1), vec3(1,0,1), vec3(1,1,1), vec3(0,1,1),
    vec3(0,0,1), vec3(0,1,0), vec3(0,0,0), vec3(0,0,1), vec3(0,1,1), vec3(0,1,0),
    vec3(1,0,0), vec3(1,1,1), vec3(1,0,1), vec3(1,0,0), vec3(1,1,0), vec3(1,1,1),
    vec3(0,1,0), vec3(1,1,1), vec3(1,1,0), vec3(0,1,0), vec3(0,1,1), vec3(1,1,1),
    vec3(0,0,1), vec3(1,0,0), vec3(1,0,1), vec3(0,0,1), vec3(0,0,0), vec3(1,0,0)
);

const vec3 faceNormals[6] = vec3[](
    vec3(0,0,-1), vec3(0,0,1), vec3(-1,0,0), vec3(1,0,0), vec3(0,1,0), vec3(0,-1,0)
);

const vec2 cornerUV[6] = vec2[](
    vec2(0,0), vec2(1,1), vec2(1,0), vec2(0,0), vec2(0,1), vec2(1,1)
);

void main()
{
    uint face = DecodeFace(vData);
    uint corner = DecodeCorner(vData);
    uint vertexIndex = face * 6u + corner;

    vec3 blockPos = vec3(float(DecodeX(vData)), float(DecodeY(vData)), float(DecodeZ(vData)));
    vec3 worldPos = blockPos + faceVerts[vertexIndex] + location;

    FragNormal = faceNormals[face];
    FragUV = cornerUV[corner];
    BlockType = DecodeBlock(vData);

    gl_Position = projection * view * vec4(worldPos, 1.0);
}
