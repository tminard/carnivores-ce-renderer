#version 330 core
layout (location = 0) in vec3 aPos;

uniform highp mat4 MVP;

void main()
{
    vec4 v = vec4(aPos, 1);
    gl_Position = MVP * v;
}
