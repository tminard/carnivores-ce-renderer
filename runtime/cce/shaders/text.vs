#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;

out vec2 TexCoord;

uniform mat4 projection; // Orthographic projection for screen coordinates

void main()
{
    gl_Position = projection * vec4(position, 0.0, 1.0);
    TexCoord = texCoord;
}