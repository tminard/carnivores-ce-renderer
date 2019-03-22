#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 uv;

out vec2 texCoord0;

uniform highp mat4 MVP;

void main()
{
    texCoord0 = uv;

    vec4 v = vec4(aPos, 1);
    gl_Position = MVP * v;
}
