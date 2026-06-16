#version 330 core

in vec3 FragNormal;

out vec4 FragColor;

void main()
{
    vec3 color = vec3(1.0);
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diffuse = max(dot(normalize(FragNormal), lightDir), 0.55);

    FragColor = vec4(color * diffuse, 1.0);
}