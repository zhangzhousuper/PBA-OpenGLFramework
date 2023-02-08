#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D diffuseTexture1;

void main()
{
    FragColor = texture(diffuseTexture1, TexCoords);
    // FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    return;
}