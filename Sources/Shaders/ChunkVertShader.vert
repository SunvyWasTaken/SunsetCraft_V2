#version 330 core

layout(location = 0) in uint vData;

uniform vec3 location;
uniform mat4 projection;
uniform mat4 view;

out vec3 FragNormal;
out vec2 FragUV;
flat out uint BlockType;

vec3 DecodePos(uint v)
{
    return vec3(
        float(v & 31u),
        float((v >> 5) & 31u),
        float((v >> 10) & 31u)
    );
}

const vec3 cubeVerts[36] = vec3[](
    // Front
    vec3(0,0,0), vec3(1,0,0), vec3(1,1,0),
    vec3(0,0,0), vec3(1,1,0), vec3(0,1,0),

    // Back
    vec3(1,0,1), vec3(0,0,1), vec3(0,1,1),
    vec3(1,0,1), vec3(0,1,1), vec3(1,1,1),

    // Left
    vec3(0,0,1), vec3(0,0,0), vec3(0,1,0),
    vec3(0,0,1), vec3(0,1,0), vec3(0,1,1),

    // Right
    vec3(1,0,0), vec3(1,0,1), vec3(1,1,1),
    vec3(1,0,0), vec3(1,1,1), vec3(1,1,0),

    // Top
    vec3(0,1,0), vec3(1,1,0), vec3(1,1,1),
    vec3(0,1,0), vec3(1,1,1), vec3(0,1,1),

    // Bottom
    vec3(0,0,1), vec3(1,0,1), vec3(1,0,0),
    vec3(0,0,1), vec3(1,0,0), vec3(0,0,0)
);

const vec3 cubeNormals[36] = vec3[](
    vec3(0,0,-1), vec3(0,0,-1), vec3(0,0,-1),
    vec3(0,0,-1), vec3(0,0,-1), vec3(0,0,-1),

    vec3(0,0,1), vec3(0,0,1), vec3(0,0,1),
    vec3(0,0,1), vec3(0,0,1), vec3(0,0,1),

    vec3(-1,0,0), vec3(-1,0,0), vec3(-1,0,0),
    vec3(-1,0,0), vec3(-1,0,0), vec3(-1,0,0),

    vec3(1,0,0), vec3(1,0,0), vec3(1,0,0),
    vec3(1,0,0), vec3(1,0,0), vec3(1,0,0),

    vec3(0,1,0), vec3(0,1,0), vec3(0,1,0),
    vec3(0,1,0), vec3(0,1,0), vec3(0,1,0),

    vec3(0,-1,0), vec3(0,-1,0), vec3(0,-1,0),
    vec3(0,-1,0), vec3(0,-1,0), vec3(0,-1,0)
);

const vec2 cubeUV[36] = vec2[](
    vec2(0,0), vec2(1,0), vec2(1,1),
    vec2(0,0), vec2(1,1), vec2(0,1),

    vec2(0,0), vec2(1,0), vec2(1,1),
    vec2(0,0), vec2(1,1), vec2(0,1),

    vec2(0,0), vec2(1,0), vec2(1,1),
    vec2(0,0), vec2(1,1), vec2(0,1),

    vec2(0,0), vec2(1,0), vec2(1,1),
    vec2(0,0), vec2(1,1), vec2(0,1),

    vec2(0,0), vec2(1,0), vec2(1,1),
    vec2(0,0), vec2(1,1), vec2(0,1),

    vec2(0,0), vec2(1,0), vec2(1,1),
    vec2(0,0), vec2(1,1), vec2(0,1)
);

void main()
{
    vec3 blockPos = DecodePos(vData);

    int vertexIndex = gl_VertexID;

    vec3 localPos = cubeVerts[vertexIndex];

    vec3 worldPos = blockPos + localPos + location;

    FragNormal = cubeNormals[vertexIndex];
    FragUV = cubeUV[vertexIndex];

    gl_Position = projection * view * vec4(worldPos, 1.0);
}