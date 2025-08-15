#version 330 core
layout(location = 0) in vec3 position; // Base position of the vertex, if applicable
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in float faceAlpha;

out vec2 TexCoord;

uniform mat4 MVP;

void main()
{
    gl_Position = MVP * vec4(position, 1.0);
    TexCoord = texCoord;
}
