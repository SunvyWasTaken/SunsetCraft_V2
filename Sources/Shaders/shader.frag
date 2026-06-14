#version 330 core

in vec3 FragNormal;
in vec2 FragUV;
flat in uint BlockType;

out vec4 FragColor;

vec3 BlockColor(uint blockType)
{
    if (blockType == 1u)
        return vec3(0.8, 0.8, 0.8);

    return vec3(1.0, 0.0, 1.0);
}

void main()
{
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diffuse = max(dot(normalize(FragNormal), lightDir), 0.55);
    vec3 color = BlockColor(BlockType);

    FragColor = vec4(color * diffuse, 1.0);
}
