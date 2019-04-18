#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 3) in float brightness; // 0 = max bright; 55 = max dark

out vec2 texCoord0;
smooth out float brightness0;

uniform mat4 MVP;

void main()
{
    vec4 v = vec4(position, 1);
    gl_Position = MVP * v;
    texCoord0 = texCoord;
    brightness0 = brightness;
}
