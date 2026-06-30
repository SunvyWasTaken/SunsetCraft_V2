#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_UV;
layout(location = 3) in uint a_UVIndex;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 v_Normal;
out vec2 v_UV;
flat out uint v_UVIndex;

void main()
{
    v_Normal = mat3(u_Model) * a_Normal;
    v_UV = a_UV;
    v_UVIndex = a_UVIndex;

    gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0);
}