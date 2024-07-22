#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 uv;
layout (location = 3) in mat4 instancedMatrix;

out vec2 texCoord0;

uniform mat4 MVP;
uniform mat4 projection_view;

void main()
{
    vec4 v = vec4(aPos, 1);
    vec4 pos = projection_view * instancedMatrix * v;

    texCoord0 = uv;
    gl_Position = pos;
}
